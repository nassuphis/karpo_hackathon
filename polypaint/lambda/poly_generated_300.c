/* AUTO-GENERATED from poly300.py — do not edit manually */
/* 100 coefficient functions */

static void poly_201_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    /* j = np.arange(0, 71) — loop variable */
    for (int j = 0; j < 71; j++) {
        double _re1r = 0, _re1i = 0;
        _re1r = x1r; _re1i = 0;
        double _pow2r = 0, _pow2i = 0;
        c_powr(_re1r, _re1i, j, &_pow2r, &_pow2i);
        double _ang3r = 0, _ang3i = 0;
        _ang3r = c_arg(x2r, x2i); _ang3i = 0;
        double _mul4r = 0, _mul4i = 0;
        c_mul(j, 0, _ang3r, _ang3i, &_mul4r, &_mul4i);
        double _sin5r = 0, _sin5i = 0;
        c_sin(_mul4r, _mul4i, &_sin5r, &_sin5i);
        double _mul6r = 0, _mul6i = 0;
        c_mul(_pow2r, _pow2i, _sin5r, _sin5i, &_mul6r, &_mul6i);
        double _re7r = 0, _re7i = 0;
        _re7r = x2r; _re7i = 0;
        double _pow8r = 0, _pow8i = 0;
        c_powr(_re7r, _re7i, j, &_pow8r, &_pow8i);
        double _ang9r = 0, _ang9i = 0;
        _ang9r = c_arg(x1r, x1i); _ang9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(j, 0, _ang9r, _ang9i, &_mul10r, &_mul10i);
        double _cos11r = 0, _cos11i = 0;
        c_cos(_mul10r, _mul10i, &_cos11r, &_cos11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_pow8r, _pow8i, _cos11r, _cos11i, &_mul12r, &_mul12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _mul6r + _mul12r; _add13i = _mul6i + _mul12i;
        double _im14r = 0, _im14i = 0;
        _im14r = x1i; _im14i = 0;
        double _pow15r = 0, _pow15i = 0;
        c_powr(_im14r, _im14i, j, &_pow15r, &_pow15i);
        double _ang16r = 0, _ang16i = 0;
        _ang16r = c_arg(x2r, x2i); _ang16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(j, 0, _ang16r, _ang16i, &_mul17r, &_mul17i);
        double _cos18r = 0, _cos18i = 0;
        c_cos(_mul17r, _mul17i, &_cos18r, &_cos18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_pow15r, _pow15i, _cos18r, _cos18i, &_mul19r, &_mul19i);
        double _im20r = 0, _im20i = 0;
        _im20r = x2i; _im20i = 0;
        double _pow21r = 0, _pow21i = 0;
        c_powr(_im20r, _im20i, j, &_pow21r, &_pow21i);
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x1r, x1i); _ang22i = 0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(j, 0, _ang22r, _ang22i, &_mul23r, &_mul23i);
        double _sin24r = 0, _sin24i = 0;
        c_sin(_mul23r, _mul23i, &_sin24r, &_sin24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_pow21r, _pow21i, _sin24r, _sin24i, &_mul25r, &_mul25i);
        double _sub26r = 0, _sub26i = 0;
        _sub26r = _mul19r - _mul25r; _sub26i = _mul19i - _mul25i;
        double _c27r = 0, _c27i = 0;
        _c27r = 0.0; _c27i = 1.0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(_sub26r, _sub26i, _c27r, _c27i, &_mul28r, &_mul28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _add13r + _mul28r; _add29i = _add13i + _mul28i;
        if (j < 71) { cRe[j] = _add29r; cIm[j] = _add29i; }
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_202_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    /* k = np.arange(0, 71) — loop variable */
    for (int k = 0; k < 71; k++) {
        double _pow1r = 0, _pow1i = 0;
        c_powr(x1r, x1i, k, &_pow1r, &_pow1i);
        double _ang2r = 0, _ang2i = 0;
        _ang2r = c_arg(x1r, x1i); _ang2i = 0;
        double _mul3r = 0, _mul3i = 0;
        c_mul(k, 0, _ang2r, _ang2i, &_mul3r, &_mul3i);
        double _re4r = 0, _re4i = 0;
        _re4r = x2r; _re4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = _mul3r + _re4r; _add5i = _mul3i + _re4i;
        double _sin6r = 0, _sin6i = 0;
        c_sin(_add5r, _add5i, &_sin6r, &_sin6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(_pow1r, _pow1i, _sin6r, _sin6i, &_mul7r, &_mul7i);
        double _pow8r = 0, _pow8i = 0;
        c_powr(x2r, x2i, k, &_pow8r, &_pow8i);
        double _ang9r = 0, _ang9i = 0;
        _ang9r = c_arg(x2r, x2i); _ang9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(k, 0, _ang9r, _ang9i, &_mul10r, &_mul10i);
        double _re11r = 0, _re11i = 0;
        _re11r = x1r; _re11i = 0;
        double _sub12r = 0, _sub12i = 0;
        _sub12r = _mul10r - _re11r; _sub12i = _mul10i - _re11i;
        double _cos13r = 0, _cos13i = 0;
        c_cos(_sub12r, _sub12i, &_cos13r, &_cos13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_pow8r, _pow8i, _cos13r, _cos13i, &_mul14r, &_mul14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _mul7r + _mul14r; _add15i = _mul7i + _mul14i;
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
        double _mul21r = 0, _mul21i = 0;
        c_mul(_add15r, _add15i, _log20r, _log20i, &_mul21r, &_mul21i);
        double _c22r = 0, _c22i = 0;
        _c22r = 1.0; _c22i = 0;
        double _c23r = 0, _c23i = 0;
        _c23r = 2.0; _c23i = 0;
        double _pow24r = 0, _pow24i = 0;
        c_mul(k, 0, k, 0, &_pow24r, &_pow24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _c22r + _pow24r; _add25i = _c22i + _pow24i;
        double _div26r = 0, _div26i = 0;
        c_div(_mul21r, _mul21i, _add25r, _add25i, &_div26r, &_div26i);
        double _re27r = 0, _re27i = 0;
        _re27r = x1r; _re27i = 0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(k, 0, _re27r, _re27i, &_mul28r, &_mul28i);
        double _sin29r = 0, _sin29i = 0;
        c_sin(_mul28r, _mul28i, &_sin29r, &_sin29i);
        double _im30r = 0, _im30i = 0;
        _im30r = x2i; _im30i = 0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(k, 0, _im30r, _im30i, &_mul31r, &_mul31i);
        double _cos32r = 0, _cos32i = 0;
        c_cos(_mul31r, _mul31i, &_cos32r, &_cos32i);
        double _sub33r = 0, _sub33i = 0;
        _sub33r = _sin29r - _cos32r; _sub33i = _sin29i - _cos32i;
        double _abs34r = 0, _abs34i = 0;
        _abs34r = c_abs(x1r, x1i); _abs34i = 0;
        double _abs35r = 0, _abs35i = 0;
        _abs35r = c_abs(x2r, x2i); _abs35i = 0;
        double _add36r = 0, _add36i = 0;
        _add36r = _abs34r + _abs35r; _add36i = _abs34i + _abs35i;
        double _mul37r = 0, _mul37i = 0;
        c_mul(_sub33r, _sub33i, _add36r, _add36i, &_mul37r, &_mul37i);
        double _c38r = 0, _c38i = 0;
        _c38r = 2.0; _c38i = 0;
        double _add39r = 0, _add39i = 0;
        _add39r = _c38r + k; _add39i = _c38i + 0;
        double _div40r = 0, _div40i = 0;
        c_div(_mul37r, _mul37i, _add39r, _add39i, &_div40r, &_div40i);
        double _add41r = 0, _add41i = 0;
        _add41r = _div26r + _div40r; _add41i = _div26i + _div40i;
        if (k < 71) { cRe[k] = _add41r; cIm[k] = _add41i; }
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_203_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    /* k = np.arange(1, 72) — loop variable */
    for (int k = 1; k < 72; k++) {
        double _re1r = 0, _re1i = 0;
        _re1r = x1r; _re1i = 0;
        double _pow2r = 0, _pow2i = 0;
        c_powr(_re1r, _re1i, k, &_pow2r, &_pow2i);
        double _ang3r = 0, _ang3i = 0;
        _ang3r = c_arg(x2r, x2i); _ang3i = 0;
        double _mul4r = 0, _mul4i = 0;
        c_mul(k, 0, _ang3r, _ang3i, &_mul4r, &_mul4i);
        double _sin5r = 0, _sin5i = 0;
        c_sin(_mul4r, _mul4i, &_sin5r, &_sin5i);
        double _mul6r = 0, _mul6i = 0;
        c_mul(_pow2r, _pow2i, _sin5r, _sin5i, &_mul6r, &_mul6i);
        double _re7r = 0, _re7i = 0;
        _re7r = x2r; _re7i = 0;
        double _pow8r = 0, _pow8i = 0;
        c_powr(_re7r, _re7i, k, &_pow8r, &_pow8i);
        double _ang9r = 0, _ang9i = 0;
        _ang9r = c_arg(x1r, x1i); _ang9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(k, 0, _ang9r, _ang9i, &_mul10r, &_mul10i);
        double _cos11r = 0, _cos11i = 0;
        c_cos(_mul10r, _mul10i, &_cos11r, &_cos11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_pow8r, _pow8i, _cos11r, _cos11i, &_mul12r, &_mul12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _mul6r + _mul12r; _add13i = _mul6i + _mul12i;
        double _abs14r = 0, _abs14i = 0;
        _abs14r = c_abs(x1r, x1i); _abs14i = 0;
        double _add15r = 0, _add15i = 0;
        _add15r = _abs14r + k; _add15i = _abs14i + 0;
        double _log16r = 0, _log16i = 0;
        c_log(_add15r, _add15i, &_log16r, &_log16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _add13r + _log16r; _add17i = _add13i + _log16i;
        double _add18r = 0, _add18i = 0;
        _add18r = x1r + x2r; _add18i = x1i + x2i;
        double _re19r = 0, _re19i = 0;
        _re19r = _add18r; _re19i = 0;
        double _pow20r = 0, _pow20i = 0;
        c_powr(_re19r, _re19i, k, &_pow20r, &_pow20i);
        double _c21r = 0, _c21i = 0;
        _c21r = 1.0; _c21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = k + _c21r; _add22i = 0 + _c21i;
        double _div23r = 0, _div23i = 0;
        c_div(_pow20r, _pow20i, _add22r, _add22i, &_div23r, &_div23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _add17r + _div23r; _add24i = _add17i + _div23i;
        double real_part = _add24r; double real_part_im = _add24i;
        double _im25r = 0, _im25i = 0;
        _im25r = x1i; _im25i = 0;
        double _pow26r = 0, _pow26i = 0;
        c_powr(_im25r, _im25i, k, &_pow26r, &_pow26i);
        double _ang27r = 0, _ang27i = 0;
        _ang27r = c_arg(x2r, x2i); _ang27i = 0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(k, 0, _ang27r, _ang27i, &_mul28r, &_mul28i);
        double _cos29r = 0, _cos29i = 0;
        c_cos(_mul28r, _mul28i, &_cos29r, &_cos29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_pow26r, _pow26i, _cos29r, _cos29i, &_mul30r, &_mul30i);
        double _im31r = 0, _im31i = 0;
        _im31r = x2i; _im31i = 0;
        double _pow32r = 0, _pow32i = 0;
        c_powr(_im31r, _im31i, k, &_pow32r, &_pow32i);
        double _ang33r = 0, _ang33i = 0;
        _ang33r = c_arg(x1r, x1i); _ang33i = 0;
        double _mul34r = 0, _mul34i = 0;
        c_mul(k, 0, _ang33r, _ang33i, &_mul34r, &_mul34i);
        double _sin35r = 0, _sin35i = 0;
        c_sin(_mul34r, _mul34i, &_sin35r, &_sin35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_pow32r, _pow32i, _sin35r, _sin35i, &_mul36r, &_mul36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _mul30r + _mul36r; _add37i = _mul30i + _mul36i;
        double _sin38r = 0, _sin38i = 0;
        c_sin(k, 0, &_sin38r, &_sin38i);
        double _add39r = 0, _add39i = 0;
        _add39r = _add37r + _sin38r; _add39i = _add37i + _sin38i;
        double _cos40r = 0, _cos40i = 0;
        c_cos(k, 0, &_cos40r, &_cos40i);
        double _add41r = 0, _add41i = 0;
        _add41r = _add39r + _cos40r; _add41i = _add39i + _cos40i;
        double imag_part = _add41r; double imag_part_im = _add41i;
        double _c42r = 0, _c42i = 0;
        _c42r = 0.0; _c42i = 1.0;
        double _mul43r = 0, _mul43i = 0;
        c_mul(_c42r, _c42i, imag_part, 0, &_mul43r, &_mul43i);
        double _add44r = 0, _add44i = 0;
        _add44r = real_part + _mul43r; _add44i = 0 + _mul43i;
        if ((k - 1) < 71) { cRe[(k - 1)] = _add44r; cIm[(k - 1)] = _add44i; }
        if ((k - 1) < 71) {
            double _cf_r = cRe[(k - 1)], _cf_i = cIm[(k - 1)];
            double _c45r = 0, _c45i = 0;
            _c45r = 1.0; _c45i = 0;
            double _neg46r = 0, _neg46i = 0;
            _neg46r = -(_c45r); _neg46i = -(_c45i);
            double _pow47r = 0, _pow47i = 0;
            c_powr(_neg46r, _neg46i, k, &_pow47r, &_pow47i);
            double _abs48r = 0, _abs48i = 0;
            _abs48r = c_abs(x1r, x1i); _abs48i = 0;
            double _add49r = 0, _add49i = 0;
            _add49r = k + _abs48r; _add49i = 0 + _abs48i;
            double _log50r = 0, _log50i = 0;
            c_log(_add49r, _add49i, &_log50r, &_log50i);
            double _mul51r = 0, _mul51i = 0;
            c_mul(_pow47r, _pow47i, _log50r, _log50i, &_mul51r, &_mul51i);
            double _mul52r = 0, _mul52i = 0;
            c_mul(_cf_r, _cf_i, _mul51r, _mul51i, &_mul52r, &_mul52i);
            cRe[(k - 1)] = _mul52r; cIm[(k - 1)] = _mul52i;
        }
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_204_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    /* j = np.arange(1, 72) — loop variable */
    for (int j = 1; j < 72; j++) {
        double _re1r = 0, _re1i = 0;
        _re1r = x1r; _re1i = 0;
        double _pow2r = 0, _pow2i = 0;
        c_powr(_re1r, _re1i, j, &_pow2r, &_pow2i);
        double _re3r = 0, _re3i = 0;
        _re3r = x2r; _re3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 71.0; _c4i = 0;
        double _sub5r = 0, _sub5i = 0;
        _sub5r = _c4r - j; _sub5i = _c4i - 0;
        double _pow6r = 0, _pow6i = 0;
        c_powc(_re3r, _re3i, _sub5r, _sub5i, &_pow6r, &_pow6i);
        double _ang7r = 0, _ang7i = 0;
        _ang7r = c_arg(x1r, x1i); _ang7i = 0;
        double _mul8r = 0, _mul8i = 0;
        c_mul(j, 0, _ang7r, _ang7i, &_mul8r, &_mul8i);
        double _c9r = 0, _c9i = 0;
        _c9r = 71.0; _c9i = 0;
        double _sub10r = 0, _sub10i = 0;
        _sub10r = _c9r - j; _sub10i = _c9i - 0;
        double _ang11r = 0, _ang11i = 0;
        _ang11r = c_arg(x2r, x2i); _ang11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(_sub10r, _sub10i, _ang11r, _ang11i, &_mul12r, &_mul12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _mul8r + _mul12r; _add13i = _mul8i + _mul12i;
        double _cos14r = 0, _cos14i = 0;
        c_cos(_add13r, _add13i, &_cos14r, &_cos14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_pow6r, _pow6i, _cos14r, _cos14i, &_mul15r, &_mul15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _pow2r + _mul15r; _add16i = _pow2i + _mul15i;
        double _abs17r = 0, _abs17i = 0;
        _abs17r = c_abs(x1r, x1i); _abs17i = 0;
        double _c18r = 0, _c18i = 0;
        _c18r = 1.0; _c18i = 0;
        double _add19r = 0, _add19i = 0;
        _add19r = _abs17r + _c18r; _add19i = _abs17i + _c18i;
        double _log20r = 0, _log20i = 0;
        c_log(_add19r, _add19i, &_log20r, &_log20i);
        double _ang21r = 0, _ang21i = 0;
        _ang21r = c_arg(x2r, x2i); _ang21i = 0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(j, 0, _ang21r, _ang21i, &_mul22r, &_mul22i);
        double _sin23r = 0, _sin23i = 0;
        c_sin(_mul22r, _mul22i, &_sin23r, &_sin23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_log20r, _log20i, _sin23r, _sin23i, &_mul24r, &_mul24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _add16r + _mul24r; _add25i = _add16i + _mul24i;
        if ((j - 1) < 71) { cRe[(j - 1)] = _add25r; cIm[(j - 1)] = _add25i; }
        double _im26r = 0, _im26i = 0;
        _im26r = x1i; _im26i = 0;
        double _pow27r = 0, _pow27i = 0;
        c_powr(_im26r, _im26i, j, &_pow27r, &_pow27i);
        double _im28r = 0, _im28i = 0;
        _im28r = x2i; _im28i = 0;
        double _c29r = 0, _c29i = 0;
        _c29r = 71.0; _c29i = 0;
        double _sub30r = 0, _sub30i = 0;
        _sub30r = _c29r - j; _sub30i = _c29i - 0;
        double _pow31r = 0, _pow31i = 0;
        c_powc(_im28r, _im28i, _sub30r, _sub30i, &_pow31r, &_pow31i);
        double _ang32r = 0, _ang32i = 0;
        _ang32r = c_arg(x1r, x1i); _ang32i = 0;
        double _mul33r = 0, _mul33i = 0;
        c_mul(j, 0, _ang32r, _ang32i, &_mul33r, &_mul33i);
        double _c34r = 0, _c34i = 0;
        _c34r = 71.0; _c34i = 0;
        double _sub35r = 0, _sub35i = 0;
        _sub35r = _c34r - j; _sub35i = _c34i - 0;
        double _ang36r = 0, _ang36i = 0;
        _ang36r = c_arg(x2r, x2i); _ang36i = 0;
        double _mul37r = 0, _mul37i = 0;
        c_mul(_sub35r, _sub35i, _ang36r, _ang36i, &_mul37r, &_mul37i);
        double _sub38r = 0, _sub38i = 0;
        _sub38r = _mul33r - _mul37r; _sub38i = _mul33i - _mul37i;
        double _sin39r = 0, _sin39i = 0;
        c_sin(_sub38r, _sub38i, &_sin39r, &_sin39i);
        double _mul40r = 0, _mul40i = 0;
        c_mul(_pow31r, _pow31i, _sin39r, _sin39i, &_mul40r, &_mul40i);
        double _sub41r = 0, _sub41i = 0;
        _sub41r = _pow27r - _mul40r; _sub41i = _pow27i - _mul40i;
        double _abs42r = 0, _abs42i = 0;
        _abs42r = c_abs(x2r, x2i); _abs42i = 0;
        double _c43r = 0, _c43i = 0;
        _c43r = 1.0; _c43i = 0;
        double _add44r = 0, _add44i = 0;
        _add44r = _abs42r + _c43r; _add44i = _abs42i + _c43i;
        double _log45r = 0, _log45i = 0;
        c_log(_add44r, _add44i, &_log45r, &_log45i);
        double _ang46r = 0, _ang46i = 0;
        _ang46r = c_arg(x1r, x1i); _ang46i = 0;
        double _mul47r = 0, _mul47i = 0;
        c_mul(j, 0, _ang46r, _ang46i, &_mul47r, &_mul47i);
        double _cos48r = 0, _cos48i = 0;
        c_cos(_mul47r, _mul47i, &_cos48r, &_cos48i);
        double _mul49r = 0, _mul49i = 0;
        c_mul(_log45r, _log45i, _cos48r, _cos48i, &_mul49r, &_mul49i);
        double _add50r = 0, _add50i = 0;
        _add50r = _sub41r + _mul49r; _add50i = _sub41i + _mul49i;
        if ((j - 1) < 71) { cRe[(j - 1)] += _add50r; cIm[(j - 1)] += _add50i; }
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_205_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 0; j < 71; j++) {
        double _re1r = 0, _re1i = 0;
        _re1r = x1r; _re1i = 0;
        double _c2r = 0, _c2i = 0;
        _c2r = 1.0; _c2i = 0;
        double _add3r = 0, _add3i = 0;
        _add3r = j + _c2r; _add3i = 0 + _c2i;
        double _c4r = 0, _c4i = 0;
        _c4r = 2.0; _c4i = 0;
        double _pow5r = 0, _pow5i = 0;
        c_mul(_add3r, _add3i, _add3r, _add3i, &_pow5r, &_pow5i);
        double _mul6r = 0, _mul6i = 0;
        c_mul(_re1r, _re1i, _pow5r, _pow5i, &_mul6r, &_mul6i);
        double _im7r = 0, _im7i = 0;
        _im7r = x2i; _im7i = 0;
        double _c8r = 0, _c8i = 0;
        _c8r = 1.0; _c8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = j + _c8r; _add9i = 0 + _c8i;
        double _mul10r = 0, _mul10i = 0;
        c_mul(_im7r, _im7i, _add9r, _add9i, &_mul10r, &_mul10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _mul6r + _mul10r; _add11i = _mul6i + _mul10i;
        double _c12r = 0, _c12i = 0;
        _c12r = 1.0; _c12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = j + _c12r; _add13i = 0 + _c12i;
        double _add14r = 0, _add14i = 0;
        _add14r = x1r + x2r; _add14i = x1i + x2i;
        double _ang15r = 0, _ang15i = 0;
        _ang15r = c_arg(_add14r, _add14i); _ang15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(_add13r, _add13i, _ang15r, _ang15i, &_mul16r, &_mul16i);
        double _sin17r = 0, _sin17i = 0;
        c_sin(_mul16r, _mul16i, &_sin17r, &_sin17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_add11r, _add11i, _sin17r, _sin17i, &_mul18r, &_mul18i);
        double _c19r = 0, _c19i = 0;
        _c19r = 1.0; _c19i = 0;
        double _add20r = 0, _add20i = 0;
        _add20r = j + _c19r; _add20i = 0 + _c19i;
        double _ang21r = 0, _ang21i = 0;
        _ang21r = c_arg(x1r, x1i); _ang21i = 0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(_add20r, _add20i, _ang21r, _ang21i, &_mul22r, &_mul22i);
        double _cos23r = 0, _cos23i = 0;
        c_cos(_mul22r, _mul22i, &_cos23r, &_cos23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul24r, &_mul24i);
        double _abs25r = 0, _abs25i = 0;
        _abs25r = c_abs(_mul24r, _mul24i); _abs25i = 0;
        double _c26r = 0, _c26i = 0;
        _c26r = 1.0; _c26i = 0;
        double _add27r = 0, _add27i = 0;
        _add27r = _abs25r + _c26r; _add27i = _abs25i + _c26i;
        double _log28r = 0, _log28i = 0;
        c_log(_add27r, _add27i, &_log28r, &_log28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _cos23r + _log28r; _add29i = _cos23i + _log28i;
        double _c30r = 0, _c30i = 0;
        _c30r = 1.0; _c30i = 0;
        double _add31r = 0, _add31i = 0;
        _add31r = j + _c30r; _add31i = 0 + _c30i;
        double _cos32r = 0, _cos32i = 0;
        c_cos(_add31r, _add31i, &_cos32r, &_cos32i);
        double _c33r = 0, _c33i = 0;
        _c33r = 1.0; _c33i = 0;
        double _add34r = 0, _add34i = 0;
        _add34r = j + _c33r; _add34i = 0 + _c33i;
        double _sin35r = 0, _sin35i = 0;
        c_sin(_add34r, _add34i, &_sin35r, &_sin35i);
        double _c36r = 0, _c36i = 0;
        _c36r = 0.0; _c36i = 0;
        double _c37r = 0, _c37i = 0;
        _c37r = 0.0; _c37i = 1.0;
        double _add38r = 0, _add38i = 0;
        _add38r = _c36r + _c37r; _add38i = _c36i + _c37i;
        double _mul39r = 0, _mul39i = 0;
        c_mul(_sin35r, _sin35i, _add38r, _add38i, &_mul39r, &_mul39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _cos32r + _mul39r; _add40i = _cos32i + _mul39i;
        double _mul41r = 0, _mul41i = 0;
        c_mul(_add29r, _add29i, _add40r, _add40i, &_mul41r, &_mul41i);
        double _add42r = 0, _add42i = 0;
        _add42r = _mul18r + _mul41r; _add42i = _mul18i + _mul41i;
        { int _idx = j; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add42r; cIm[_idx] = _add42i; } }
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_206_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 0; j < 71; j++) {
        double _re1r = 0, _re1i = 0;
        _re1r = x1r; _re1i = 0;
        double _c2r = 0, _c2i = 0;
        _c2r = 1.0; _c2i = 0;
        double _add3r = 0, _add3i = 0;
        _add3r = j + _c2r; _add3i = 0 + _c2i;
        double _pow4r = 0, _pow4i = 0;
        c_powc(_re1r, _re1i, _add3r, _add3i, &_pow4r, &_pow4i);
        double _c5r = 0, _c5i = 0;
        _c5r = 1.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = j + _c5r; _add6i = 0 + _c5i;
        double _ang7r = 0, _ang7i = 0;
        _ang7r = c_arg(x2r, x2i); _ang7i = 0;
        double _mul8r = 0, _mul8i = 0;
        c_mul(_add6r, _add6i, _ang7r, _ang7i, &_mul8r, &_mul8i);
        double _abs9r = 0, _abs9i = 0;
        _abs9r = c_abs(x1r, x1i); _abs9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _abs9r + _c10r; _add11i = _abs9i + _c10i;
        double _log12r = 0, _log12i = 0;
        c_log(_add11r, _add11i, &_log12r, &_log12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _mul8r + _log12r; _add13i = _mul8i + _log12i;
        double _sin14r = 0, _sin14i = 0;
        c_sin(_add13r, _add13i, &_sin14r, &_sin14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_pow4r, _pow4i, _sin14r, _sin14i, &_mul15r, &_mul15i);
        double real_part_r = _mul15r, real_part_i = _mul15i;
        double _im16r = 0, _im16i = 0;
        _im16r = x2i; _im16i = 0;
        double _c17r = 0, _c17i = 0;
        _c17r = 1.0; _c17i = 0;
        double _add18r = 0, _add18i = 0;
        _add18r = j + _c17r; _add18i = 0 + _c17i;
        double _pow19r = 0, _pow19i = 0;
        c_powc(_im16r, _im16i, _add18r, _add18i, &_pow19r, &_pow19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 1.0; _c20i = 0;
        double _add21r = 0, _add21i = 0;
        _add21r = j + _c20r; _add21i = 0 + _c20i;
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x1r, x1i); _ang22i = 0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(_add21r, _add21i, _ang22r, _ang22i, &_mul23r, &_mul23i);
        double _abs24r = 0, _abs24i = 0;
        _abs24r = c_abs(x2r, x2i); _abs24i = 0;
        double _c25r = 0, _c25i = 0;
        _c25r = 1.0; _c25i = 0;
        double _add26r = 0, _add26i = 0;
        _add26r = _abs24r + _c25r; _add26i = _abs24i + _c25i;
        double _log27r = 0, _log27i = 0;
        c_log(_add26r, _add26i, &_log27r, &_log27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _mul23r + _log27r; _add28i = _mul23i + _log27i;
        double _cos29r = 0, _cos29i = 0;
        c_cos(_add28r, _add28i, &_cos29r, &_cos29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_pow19r, _pow19i, _cos29r, _cos29i, &_mul30r, &_mul30i);
        double imag_part_r = _mul30r, imag_part_i = _mul30i;
        double _im31r = 0, _im31i = 0;
        _im31r = x1i; _im31i = 0;
        double _re32r = 0, _re32i = 0;
        _re32r = x2r; _re32i = 0;
        double _mul33r = 0, _mul33i = 0;
        c_mul(_im31r, _im31i, _re32r, _re32i, &_mul33r, &_mul33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(_mul33r, _mul33i, imag_part_r, imag_part_i, &_mul34r, &_mul34i);
        double _add35r = 0, _add35i = 0;
        _add35r = real_part_r + _mul34r; _add35i = real_part_i + _mul34i;
        { int _idx = j; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add35r; cIm[_idx] = _add35i; } }
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_207_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 0; j < 71; j++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 1.0; _c1i = 0;
        double _add2r = 0, _add2i = 0;
        _add2r = j + _c1r; _add2i = 0 + _c1i;
        double _c3r = 0, _c3i = 0;
        _c3r = 2.0; _c3i = 0;
        double _pow4r = 0, _pow4i = 0;
        c_mul(_add2r, _add2i, _add2r, _add2i, &_pow4r, &_pow4i);
        double _re5r = 0, _re5i = 0;
        _re5r = x1r; _re5i = 0;
        double _mul6r = 0, _mul6i = 0;
        c_mul(_pow4r, _pow4i, _re5r, _re5i, &_mul6r, &_mul6i);
        double _im7r = 0, _im7i = 0;
        _im7r = x2i; _im7i = 0;
        double _sub8r = 0, _sub8i = 0;
        _sub8r = _mul6r - _im7r; _sub8i = _mul6i - _im7i;
        double k_r = _sub8r, k_i = _sub8i;
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = j + _c9r; _add10i = 0 + _c9i;
        double _ang11r = 0, _ang11i = 0;
        _ang11r = c_arg(x1r, x1i); _ang11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(_add10r, _add10i, _ang11r, _ang11i, &_mul12r, &_mul12i);
        double _sin13r = 0, _sin13i = 0;
        c_sin(_mul12r, _mul12i, &_sin13r, &_sin13i);
        double _c14r = 0, _c14i = 0;
        _c14r = 1.0; _c14i = 0;
        double _add15r = 0, _add15i = 0;
        _add15r = j + _c14r; _add15i = 0 + _c14i;
        double _ang16r = 0, _ang16i = 0;
        _ang16r = c_arg(x2r, x2i); _ang16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(_add15r, _add15i, _ang16r, _ang16i, &_mul17r, &_mul17i);
        double _cos18r = 0, _cos18i = 0;
        c_cos(_mul17r, _mul17i, &_cos18r, &_cos18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _sin13r + _cos18r; _add19i = _sin13i + _cos18i;
        double r_r = _add19r, r_i = _add19i;
        double _abs20r = 0, _abs20i = 0;
        _abs20r = c_abs(x1r, x1i); _abs20i = 0;
        double _c21r = 0, _c21i = 0;
        _c21r = 1.0; _c21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = _abs20r + _c21r; _add22i = _abs20i + _c21i;
        double _log23r = 0, _log23i = 0;
        c_log(_add22r, _add22i, &_log23r, &_log23i);
        double _c24r = 0, _c24i = 0;
        _c24r = 2.0; _c24i = 0;
        double _pow25r = 0, _pow25i = 0;
        c_mul(r_r, r_i, r_r, r_i, &_pow25r, &_pow25i);
        double _add26r = 0, _add26i = 0;
        _add26r = k_r + _pow25r; _add26i = k_i + _pow25i;
        double _mul27r = 0, _mul27i = 0;
        c_mul(_log23r, _log23i, _add26r, _add26i, &_mul27r, &_mul27i);
        double magnitude_r = _mul27r, magnitude_i = _mul27i;
        double _abs28r = 0, _abs28i = 0;
        _abs28r = c_abs(x2r, x2i); _abs28i = 0;
        double _c29r = 0, _c29i = 0;
        _c29r = 1.0; _c29i = 0;
        double _add30r = 0, _add30i = 0;
        _add30r = j + _c29r; _add30i = 0 + _c29i;
        double _mul31r = 0, _mul31i = 0;
        c_mul(_abs28r, _abs28i, _add30r, _add30i, &_mul31r, &_mul31i);
        double _re32r = 0, _re32i = 0;
        _re32r = x1r; _re32i = 0;
        double _c33r = 0, _c33i = 0;
        _c33r = 1.0; _c33i = 0;
        double _add34r = 0, _add34i = 0;
        _add34r = j + _c33r; _add34i = 0 + _c33i;
        double _sin35r = 0, _sin35i = 0;
        c_sin(_add34r, _add34i, &_sin35r, &_sin35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_re32r, _re32i, _sin35r, _sin35i, &_mul36r, &_mul36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _mul31r + _mul36r; _add37i = _mul31i + _mul36i;
        double angle_r = _add37r, angle_i = _add37i;
        double _cos38r = 0, _cos38i = 0;
        c_cos(angle_r, angle_i, &_cos38r, &_cos38i);
        double _sin39r = 0, _sin39i = 0;
        c_sin(angle_r, angle_i, &_sin39r, &_sin39i);
        double _c40r = 0, _c40i = 0;
        _c40r = 0.0; _c40i = 1.0;
        double _mul41r = 0, _mul41i = 0;
        c_mul(_sin39r, _sin39i, _c40r, _c40i, &_mul41r, &_mul41i);
        double _add42r = 0, _add42i = 0;
        _add42r = _cos38r + _mul41r; _add42i = _cos38i + _mul41i;
        double _mul43r = 0, _mul43i = 0;
        c_mul(magnitude_r, magnitude_i, _add42r, _add42i, &_mul43r, &_mul43i);
        { int _idx = j; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul43r; cIm[_idx] = _mul43i; } }
    }
    for (int j = 0; j < 71; j++) {
        double _conj44r = 0, _conj44i = 0;
        _conj44r = x1r; _conj44i = -(x1i);
        double _conj45r = 0, _conj45i = 0;
        _conj45r = x2r; _conj45i = -(x2i);
        double _mul46r = 0, _mul46i = 0;
        c_mul(_conj44r, _conj44i, _conj45r, _conj45i, &_mul46r, &_mul46i);
        double _c47r = 0, _c47i = 0;
        _c47r = 1.0; _c47i = 0;
        double _add48r = 0, _add48i = 0;
        _add48r = j + _c47r; _add48i = 0 + _c47i;
        double _mul49r = 0, _mul49i = 0;
        c_mul(_mul46r, _mul46i, _add48r, _add48i, &_mul49r, &_mul49i);
        cRe[j] += _mul49r; cIm[j] += _mul49i;
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_208_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 0; j < 71; j++) {
        double _abs1r = 0, _abs1i = 0;
        _abs1r = c_abs(x1r, x1i); _abs1i = 0;
        double _c2r = 0, _c2i = 0;
        _c2r = 1.0; _c2i = 0;
        double _add3r = 0, _add3i = 0;
        _add3r = j + _c2r; _add3i = 0 + _c2i;
        double _mul4r = 0, _mul4i = 0;
        c_mul(_abs1r, _abs1i, _add3r, _add3i, &_mul4r, &_mul4i);
        double _abs5r = 0, _abs5i = 0;
        _abs5r = c_abs(x2r, x2i); _abs5i = 0;
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = j + _c6r; _add7i = 0 + _c6i;
        double _c8r = 0, _c8i = 0;
        _c8r = 2.0; _c8i = 0;
        double _pow9r = 0, _pow9i = 0;
        c_mul(_add7r, _add7i, _add7r, _add7i, &_pow9r, &_pow9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(_abs5r, _abs5i, _pow9r, _pow9i, &_mul10r, &_mul10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _mul4r + _mul10r; _add11i = _mul4i + _mul10i;
        double _c12r = 0, _c12i = 0;
        _c12r = 1.0; _c12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = _add11r + _c12r; _add13i = _add11i + _c12i;
        double _log14r = 0, _log14i = 0;
        c_log(_add13r, _add13i, &_log14r, &_log14i);
        double mag_r = _log14r, mag_i = _log14i;
        double _ang15r = 0, _ang15i = 0;
        _ang15r = c_arg(x1r, x1i); _ang15i = 0;
        double _c16r = 0, _c16i = 0;
        _c16r = 1.0; _c16i = 0;
        double _add17r = 0, _add17i = 0;
        _add17r = j + _c16r; _add17i = 0 + _c16i;
        double _sqrt18r = 0, _sqrt18i = 0;
        c_powr(_add17r, _add17i, 0.5, &_sqrt18r, &_sqrt18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_ang15r, _ang15i, _sqrt18r, _sqrt18i, &_mul19r, &_mul19i);
        double _ang20r = 0, _ang20i = 0;
        _ang20r = c_arg(x2r, x2i); _ang20i = 0;
        double _c21r = 0, _c21i = 0;
        _c21r = 2.0; _c21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = j + _c21r; _add22i = 0 + _c21i;
        double _log23r = 0, _log23i = 0;
        c_log(_add22r, _add22i, &_log23r, &_log23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_ang20r, _ang20i, _log23r, _log23i, &_mul24r, &_mul24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _mul19r + _mul24r; _add25i = _mul19i + _mul24i;
        double angle_r = _add25r, angle_i = _add25i;
        double _cos26r = 0, _cos26i = 0;
        c_cos(angle_r, angle_i, &_cos26r, &_cos26i);
        double _sin27r = 0, _sin27i = 0;
        c_sin(angle_r, angle_i, &_sin27r, &_sin27i);
        double _c28r = 0, _c28i = 0;
        _c28r = 0.0; _c28i = 1.0;
        double _mul29r = 0, _mul29i = 0;
        c_mul(_sin27r, _sin27i, _c28r, _c28i, &_mul29r, &_mul29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _cos26r + _mul29r; _add30i = _cos26i + _mul29i;
        double _mul31r = 0, _mul31i = 0;
        c_mul(mag_r, mag_i, _add30r, _add30i, &_mul31r, &_mul31i);
        { int _idx = j; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul31r; cIm[_idx] = _mul31i; } }
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_209_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 0; j < 71; j++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 0.0; _c1i = 0;
        double mag_r = _c1r, mag_i = _c1i;
        double _c2r = 0, _c2i = 0;
        _c2r = 0.0; _c2i = 0;
        double ang_r = _c2r, ang_i = _c2i;
        double _c3r = 0, _c3i = 0;
        _c3r = 1.0; _c3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = j + _c3r; _add4i = 0 + _c3i;
        double _c5r = 0, _c5i = 0;
        _c5r = 11.0; _c5i = 0;
        double _min6r = 0, _min6i = 0;
        _min6r = fmin(_add4r, _c5r); _min6i = 0;
        for (int k = 1; k < (int)(_min6r); k++) {
            double _re7r = 0, _re7i = 0;
            _re7r = x1r; _re7i = 0;
            double _pow8r = 0, _pow8i = 0;
            c_powr(_re7r, _re7i, k, &_pow8r, &_pow8i);
            double _re9r = 0, _re9i = 0;
            _re9r = x2r; _re9i = 0;
            double _sub10r = 0, _sub10i = 0;
            _sub10r = j - k; _sub10i = 0 - 0;
            double _pow11r = 0, _pow11i = 0;
            c_powc(_re9r, _re9i, _sub10r, _sub10i, &_pow11r, &_pow11i);
            double _mul12r = 0, _mul12i = 0;
            c_mul(_pow8r, _pow8i, _pow11r, _pow11i, &_mul12r, &_mul12i);
            double _abs13r = 0, _abs13i = 0;
            _abs13r = c_abs(x1r, x1i); _abs13i = 0;
            double _abs14r = 0, _abs14i = 0;
            _abs14r = c_abs(x2r, x2i); _abs14i = 0;
            double _add15r = 0, _add15i = 0;
            _add15r = _abs13r + _abs14r; _add15i = _abs13i + _abs14i;
            double _c16r = 0, _c16i = 0;
            _c16r = 1.0; _c16i = 0;
            double _add17r = 0, _add17i = 0;
            _add17r = _add15r + _c16r; _add17i = _add15i + _c16i;
            double _log18r = 0, _log18i = 0;
            c_log(_add17r, _add17i, &_log18r, &_log18i);
            double _mul19r = 0, _mul19i = 0;
            c_mul(_mul12r, _mul12i, _log18r, _log18i, &_mul19r, &_mul19i);
            mag += _mul19r;
            double _ang20r = 0, _ang20i = 0;
            _ang20r = c_arg(x1r, x1i); _ang20i = 0;
            double _mul21r = 0, _mul21i = 0;
            c_mul(_ang20r, _ang20i, k, 0, &_mul21r, &_mul21i);
            double _ang22r = 0, _ang22i = 0;
            _ang22r = c_arg(x2r, x2i); _ang22i = 0;
            double _sub23r = 0, _sub23i = 0;
            _sub23r = j - k; _sub23i = 0 - 0;
            double _mul24r = 0, _mul24i = 0;
            c_mul(_ang22r, _ang22i, _sub23r, _sub23i, &_mul24r, &_mul24i);
            double _sub25r = 0, _sub25i = 0;
            _sub25r = _mul21r - _mul24r; _sub25i = _mul21i - _mul24i;
            double _sin26r = 0, _sin26i = 0;
            c_sin(k, 0, &_sin26r, &_sin26i);
            double _add27r = 0, _add27i = 0;
            _add27r = x1r + x2r; _add27i = x1i + x2i;
            double _conj28r = 0, _conj28i = 0;
            _conj28r = _add27r; _conj28i = -(_add27i);
            double _ang29r = 0, _ang29i = 0;
            _ang29r = c_arg(_conj28r, _conj28i); _ang29i = 0;
            double _mul30r = 0, _mul30i = 0;
            c_mul(_sin26r, _sin26i, _ang29r, _ang29i, &_mul30r, &_mul30i);
            double _add31r = 0, _add31i = 0;
            _add31r = _sub25r + _mul30r; _add31i = _sub25i + _mul30i;
            ang += _add31r;
        }
        double _cos32r = 0, _cos32i = 0;
        c_cos(ang_r, ang_i, &_cos32r, &_cos32i);
        double _sin33r = 0, _sin33i = 0;
        c_sin(ang_r, ang_i, &_sin33r, &_sin33i);
        double _c34r = 0, _c34i = 0;
        _c34r = 0.0; _c34i = 1.0;
        double _mul35r = 0, _mul35i = 0;
        c_mul(_sin33r, _sin33i, _c34r, _c34i, &_mul35r, &_mul35i);
        double _add36r = 0, _add36i = 0;
        _add36r = _cos32r + _mul35r; _add36i = _cos32i + _mul35i;
        double _mul37r = 0, _mul37i = 0;
        c_mul(mag_r, mag_i, _add36r, _add36i, &_mul37r, &_mul37i);
        { int _idx = j; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul37r; cIm[_idx] = _mul37i; } }
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_210_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 0; j < 71; j++) {
        double _abs1r = 0, _abs1i = 0;
        _abs1r = c_abs(x1r, x1i); _abs1i = 0;
        double _c2r = 0, _c2i = 0;
        _c2r = 1.0; _c2i = 0;
        double _add3r = 0, _add3i = 0;
        _add3r = j + _c2r; _add3i = 0 + _c2i;
        double _pow4r = 0, _pow4i = 0;
        c_powc(_abs1r, _abs1i, _add3r, _add3i, &_pow4r, &_pow4i);
        double _abs5r = 0, _abs5i = 0;
        _abs5r = c_abs(x2r, x2i); _abs5i = 0;
        double _c6r = 0, _c6i = 0;
        _c6r = 70.0; _c6i = 0;
        double _sub7r = 0, _sub7i = 0;
        _sub7r = _c6r - j; _sub7i = _c6i - 0;
        double _pow8r = 0, _pow8i = 0;
        c_powc(_abs5r, _abs5i, _sub7r, _sub7i, &_pow8r, &_pow8i);
        double _add9r = 0, _add9i = 0;
        _add9r = _pow4r + _pow8r; _add9i = _pow4i + _pow8i;
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _add9r + _c10r; _add11i = _add9i + _c10i;
        double _log12r = 0, _log12i = 0;
        c_log(_add11r, _add11i, &_log12r, &_log12i);
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _c14r = 0, _c14i = 0;
        _c14r = 1.0; _c14i = 0;
        double _add15r = 0, _add15i = 0;
        _add15r = j + _c14r; _add15i = 0 + _c14i;
        double _re16r = 0, _re16i = 0;
        _re16r = x1r; _re16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(_add15r, _add15i, _re16r, _re16i, &_mul17r, &_mul17i);
        double _sin18r = 0, _sin18i = 0;
        c_sin(_mul17r, _mul17i, &_sin18r, &_sin18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _c13r + _sin18r; _add19i = _c13i + _sin18i;
        double _c20r = 0, _c20i = 0;
        _c20r = 1.0; _c20i = 0;
        double _add21r = 0, _add21i = 0;
        _add21r = j + _c20r; _add21i = 0 + _c20i;
        double _im22r = 0, _im22i = 0;
        _im22r = x2i; _im22i = 0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(_add21r, _add21i, _im22r, _im22i, &_mul23r, &_mul23i);
        double _cos24r = 0, _cos24i = 0;
        c_cos(_mul23r, _mul23i, &_cos24r, &_cos24i);
        double _sub25r = 0, _sub25i = 0;
        _sub25r = _add19r - _cos24r; _sub25i = _add19i - _cos24i;
        double _mul26r = 0, _mul26i = 0;
        c_mul(_log12r, _log12i, _sub25r, _sub25i, &_mul26r, &_mul26i);
        double mag_r = _mul26r, mag_i = _mul26i;
        double _ang27r = 0, _ang27i = 0;
        _ang27r = c_arg(x1r, x1i); _ang27i = 0;
        double _c28r = 0, _c28i = 0;
        _c28r = 1.0; _c28i = 0;
        double _add29r = 0, _add29i = 0;
        _add29r = j + _c28r; _add29i = 0 + _c28i;
        double _mul30r = 0, _mul30i = 0;
        c_mul(_ang27r, _ang27i, _add29r, _add29i, &_mul30r, &_mul30i);
        double _ang31r = 0, _ang31i = 0;
        _ang31r = c_arg(x2r, x2i); _ang31i = 0;
        double _c32r = 0, _c32i = 0;
        _c32r = 1.0; _c32i = 0;
        double _add33r = 0, _add33i = 0;
        _add33r = j + _c32r; _add33i = 0 + _c32i;
        double _c34r = 0, _c34i = 0;
        _c34r = 2.0; _c34i = 0;
        double _pow35r = 0, _pow35i = 0;
        c_mul(_add33r, _add33i, _add33r, _add33i, &_pow35r, &_pow35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_ang31r, _ang31i, _pow35r, _pow35i, &_mul36r, &_mul36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _mul30r + _mul36r; _add37i = _mul30i + _mul36i;
        double _c38r = 0, _c38i = 0;
        _c38r = 1.0; _c38i = 0;
        double _add39r = 0, _add39i = 0;
        _add39r = j + _c38r; _add39i = 0 + _c38i;
        double _re40r = 0, _re40i = 0;
        _re40r = x1r; _re40i = 0;
        double _mul41r = 0, _mul41i = 0;
        c_mul(_add39r, _add39i, _re40r, _re40i, &_mul41r, &_mul41i);
        double _sin42r = 0, _sin42i = 0;
        c_sin(_mul41r, _mul41i, &_sin42r, &_sin42i);
        double _add43r = 0, _add43i = 0;
        _add43r = _add37r + _sin42r; _add43i = _add37i + _sin42i;
        double _c44r = 0, _c44i = 0;
        _c44r = 1.0; _c44i = 0;
        double _add45r = 0, _add45i = 0;
        _add45r = j + _c44r; _add45i = 0 + _c44i;
        double _im46r = 0, _im46i = 0;
        _im46r = x2i; _im46i = 0;
        double _mul47r = 0, _mul47i = 0;
        c_mul(_add45r, _add45i, _im46r, _im46i, &_mul47r, &_mul47i);
        double _cos48r = 0, _cos48i = 0;
        c_cos(_mul47r, _mul47i, &_cos48r, &_cos48i);
        double _sub49r = 0, _sub49i = 0;
        _sub49r = _add43r - _cos48r; _sub49i = _add43i - _cos48i;
        double angle_r = _sub49r, angle_i = _sub49i;
        double _cos50r = 0, _cos50i = 0;
        c_cos(angle_r, angle_i, &_cos50r, &_cos50i);
        double _sin51r = 0, _sin51i = 0;
        c_sin(angle_r, angle_i, &_sin51r, &_sin51i);
        double _c52r = 0, _c52i = 0;
        _c52r = 0.0; _c52i = 1.0;
        double _mul53r = 0, _mul53i = 0;
        c_mul(_sin51r, _sin51i, _c52r, _c52i, &_mul53r, &_mul53i);
        double _add54r = 0, _add54i = 0;
        _add54r = _cos50r + _mul53r; _add54i = _cos50i + _mul53i;
        double _mul55r = 0, _mul55i = 0;
        c_mul(mag_r, mag_i, _add54r, _add54i, &_mul55r, &_mul55i);
        { int _idx = j; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul55r; cIm[_idx] = _mul55i; } }
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_211_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 0; j < 71; j++) {
        double _abs1r = 0, _abs1i = 0;
        _abs1r = c_abs(x1r, x1i); _abs1i = 0;
        double _c2r = 0, _c2i = 0;
        _c2r = 1.0; _c2i = 0;
        double _add3r = 0, _add3i = 0;
        _add3r = j + _c2r; _add3i = 0 + _c2i;
        double _mul4r = 0, _mul4i = 0;
        c_mul(_abs1r, _abs1i, _add3r, _add3i, &_mul4r, &_mul4i);
        double _abs5r = 0, _abs5i = 0;
        _abs5r = c_abs(x2r, x2i); _abs5i = 0;
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = j + _c6r; _add7i = 0 + _c6i;
        double _sqrt8r = 0, _sqrt8i = 0;
        c_powr(_add7r, _add7i, 0.5, &_sqrt8r, &_sqrt8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(_abs5r, _abs5i, _sqrt8r, _sqrt8i, &_mul9r, &_mul9i);
        double _add10r = 0, _add10i = 0;
        _add10r = _mul4r + _mul9r; _add10i = _mul4i + _mul9i;
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = _add10r + _c11r; _add12i = _add10i + _c11i;
        double _log13r = 0, _log13i = 0;
        c_log(_add12r, _add12i, &_log13r, &_log13i);
        double _c14r = 0, _c14i = 0;
        _c14r = 1.0; _c14i = 0;
        double _add15r = 0, _add15i = 0;
        _add15r = j + _c14r; _add15i = 0 + _c14i;
        double _re16r = 0, _re16i = 0;
        _re16r = x1r; _re16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(_add15r, _add15i, _re16r, _re16i, &_mul17r, &_mul17i);
        double _sin18r = 0, _sin18i = 0;
        c_sin(_mul17r, _mul17i, &_sin18r, &_sin18i);
        double _c19r = 0, _c19i = 0;
        _c19r = 1.0; _c19i = 0;
        double _add20r = 0, _add20i = 0;
        _add20r = j + _c19r; _add20i = 0 + _c19i;
        double _im21r = 0, _im21i = 0;
        _im21r = x2i; _im21i = 0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(_add20r, _add20i, _im21r, _im21i, &_mul22r, &_mul22i);
        double _cos23r = 0, _cos23i = 0;
        c_cos(_mul22r, _mul22i, &_cos23r, &_cos23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_sin18r, _sin18i, _cos23r, _cos23i, &_mul24r, &_mul24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _log13r + _mul24r; _add25i = _log13i + _mul24i;
        double mag_r = _add25r, mag_i = _add25i;
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(x1r, x1i); _ang26i = 0;
        double _c27r = 0, _c27i = 0;
        _c27r = 2.0; _c27i = 0;
        double _pow28r = 0, _pow28i = 0;
        c_mul(_ang26r, _ang26i, _ang26r, _ang26i, &_pow28r, &_pow28i);
        double _c29r = 0, _c29i = 0;
        _c29r = 2.0; _c29i = 0;
        double _add30r = 0, _add30i = 0;
        _add30r = j + _c29r; _add30i = 0 + _c29i;
        double _div31r = 0, _div31i = 0;
        c_div(_pow28r, _pow28i, _add30r, _add30i, &_div31r, &_div31i);
        double _ang32r = 0, _ang32i = 0;
        _ang32r = c_arg(x2r, x2i); _ang32i = 0;
        double _c33r = 0, _c33i = 0;
        _c33r = 1.0; _c33i = 0;
        double _add34r = 0, _add34i = 0;
        _add34r = j + _c33r; _add34i = 0 + _c33i;
        double _cos35r = 0, _cos35i = 0;
        c_cos(_add34r, _add34i, &_cos35r, &_cos35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_ang32r, _ang32i, _cos35r, _cos35i, &_mul36r, &_mul36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _div31r + _mul36r; _add37i = _div31i + _mul36i;
        double _mul38r = 0, _mul38i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul38r, &_mul38i);
        double _re39r = 0, _re39i = 0;
        _re39r = _mul38r; _re39i = 0;
        double _add40r = 0, _add40i = 0;
        _add40r = _add37r + _re39r; _add40i = _add37i + _re39i;
        double angle_r = _add40r, angle_i = _add40i;
        double _cos41r = 0, _cos41i = 0;
        c_cos(angle_r, angle_i, &_cos41r, &_cos41i);
        double _sin42r = 0, _sin42i = 0;
        c_sin(angle_r, angle_i, &_sin42r, &_sin42i);
        double _c43r = 0, _c43i = 0;
        _c43r = 0.0; _c43i = 1.0;
        double _mul44r = 0, _mul44i = 0;
        c_mul(_sin42r, _sin42i, _c43r, _c43i, &_mul44r, &_mul44i);
        double _add45r = 0, _add45i = 0;
        _add45r = _cos41r + _mul44r; _add45i = _cos41i + _mul44i;
        double _mul46r = 0, _mul46i = 0;
        c_mul(mag_r, mag_i, _add45r, _add45i, &_mul46r, &_mul46i);
        { int _idx = j; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul46r; cIm[_idx] = _mul46i; } }
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_212_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 0; j < 71; j++) {
        double _re1r = 0, _re1i = 0;
        _re1r = x1r; _re1i = 0;
        double _pow2r = 0, _pow2i = 0;
        c_powr(_re1r, _re1i, j, &_pow2r, &_pow2i);
        double _im3r = 0, _im3i = 0;
        _im3r = x2i; _im3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 70.0; _c4i = 0;
        double _sub5r = 0, _sub5i = 0;
        _sub5r = _c4r - j; _sub5i = _c4i - 0;
        double _pow6r = 0, _pow6i = 0;
        c_powc(_im3r, _im3i, _sub5r, _sub5i, &_pow6r, &_pow6i);
        double _add7r = 0, _add7i = 0;
        _add7r = _pow2r + _pow6r; _add7i = _pow2i + _pow6i;
        double _add8r = 0, _add8i = 0;
        _add8r = x1r + x2r; _add8i = x1i + x2i;
        double _ang9r = 0, _ang9i = 0;
        _ang9r = c_arg(_add8r, _add8i); _ang9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(j, 0, _ang9r, _ang9i, &_mul10r, &_mul10i);
        double _cos11r = 0, _cos11i = 0;
        c_cos(_mul10r, _mul10i, &_cos11r, &_cos11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_add7r, _add7i, _cos11r, _cos11i, &_mul12r, &_mul12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul13r, &_mul13i);
        double _ang14r = 0, _ang14i = 0;
        _ang14r = c_arg(_mul13r, _mul13i); _ang14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(j, 0, _ang14r, _ang14i, &_mul15r, &_mul15i);
        double _sin16r = 0, _sin16i = 0;
        c_sin(_mul15r, _mul15i, &_sin16r, &_sin16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _mul12r + _sin16r; _add17i = _mul12i + _sin16i;
        double _abs18r = 0, _abs18i = 0;
        _abs18r = c_abs(x1r, x1i); _abs18i = 0;
        double _abs19r = 0, _abs19i = 0;
        _abs19r = c_abs(x2r, x2i); _abs19i = 0;
        double _add20r = 0, _add20i = 0;
        _add20r = _abs18r + _abs19r; _add20i = _abs18i + _abs19i;
        double _c21r = 0, _c21i = 0;
        _c21r = 1.0; _c21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = _add20r + _c21r; _add22i = _add20i + _c21i;
        double _log23r = 0, _log23i = 0;
        c_log(_add22r, _add22i, &_log23r, &_log23i);
        double _pow24r = 0, _pow24i = 0;
        c_powr(_log23r, _log23i, j, &_pow24r, &_pow24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _add17r + _pow24r; _add25i = _add17i + _pow24i;
        { int _idx = j; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add25r; cIm[_idx] = _add25i; } }
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_213_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 0; j < 71; j++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 1.0; _c1i = 0;
        double _add2r = 0, _add2i = 0;
        _add2r = j + _c1r; _add2i = 0 + _c1i;
        double _c3r = 0, _c3i = 0;
        _c3r = 3.0; _c3i = 0;
        double _mul4r = 0, _mul4i = 0;
        c_mul(_add2r, _add2i, _c3r, _c3i, &_mul4r, &_mul4i);
        double _c5r = 0, _c5i = 0;
        _c5r = 7.0; _c5i = 0;
        double _fdiv6r = 0, _fdiv6i = 0;
        c_div(j, 0, _c5r, _c5i, &_fdiv6r, &_fdiv6i);
        _fdiv6r = floor(_fdiv6r); _fdiv6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = _mul4r + _fdiv6r; _add7i = _mul4i + _fdiv6i;
        double k_r = _add7r, k_i = _add7i;
        double _c8r = 0, _c8i = 0;
        _c8r = 5.0; _c8i = 0;
        double _mod9r = 0, _mod9i = 0;
        _mod9r = fmod(j, _c8r); _mod9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 2.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _mod9r + _c10r; _add11i = _mod9i + _c10i;
        double r_r = _add11r, r_i = _add11i;
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(x1r, x1i); _abs12i = 0;
        double _pow13r = 0, _pow13i = 0;
        c_powc(_abs12r, _abs12i, k_r, k_i, &_pow13r, &_pow13i);
        double _abs14r = 0, _abs14i = 0;
        _abs14r = c_abs(x2r, x2i); _abs14i = 0;
        double _pow15r = 0, _pow15i = 0;
        c_powc(_abs14r, _abs14i, r_r, r_i, &_pow15r, &_pow15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _pow13r + _pow15r; _add16i = _pow13i + _pow15i;
        double _re17r = 0, _re17i = 0;
        _re17r = x1r; _re17i = 0;
        double _im18r = 0, _im18i = 0;
        _im18r = x2i; _im18i = 0;
        double _sum19r = 0, _sum19i = 0;
        _sum19r = _re17r + _im18r; _sum19i = _re17i + _im18i;
        double _abs20r = 0, _abs20i = 0;
        _abs20r = c_abs(x1r, x1i); _abs20i = 0;
        double _c21r = 0, _c21i = 0;
        _c21r = 1.0; _c21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = _abs20r + _c21r; _add22i = _abs20i + _c21i;
        double _log23r = 0, _log23i = 0;
        c_log(_add22r, _add22i, &_log23r, &_log23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_sum19r, _sum19i, _log23r, _log23i, &_mul24r, &_mul24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _add16r + _mul24r; _add25i = _add16i + _mul24i;
        double mag_r = _add25r, mag_i = _add25i;
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(x1r, x1i); _ang26i = 0;
        double _mul27r = 0, _mul27i = 0;
        c_mul(_ang26r, _ang26i, k_r, k_i, &_mul27r, &_mul27i);
        double _ang28r = 0, _ang28i = 0;
        _ang28r = c_arg(x2r, x2i); _ang28i = 0;
        double _mul29r = 0, _mul29i = 0;
        c_mul(_ang28r, _ang28i, r_r, r_i, &_mul29r, &_mul29i);
        double _sub30r = 0, _sub30i = 0;
        _sub30r = _mul27r - _mul29r; _sub30i = _mul27i - _mul29i;
        double _c31r = 0, _c31i = 0;
        _c31r = 1.0; _c31i = 0;
        double _add32r = 0, _add32i = 0;
        _add32r = j + _c31r; _add32i = 0 + _c31i;
        double _sin33r = 0, _sin33i = 0;
        c_sin(_add32r, _add32i, &_sin33r, &_sin33i);
        double _c34r = 0, _c34i = 0;
        _c34r = 1.0; _c34i = 0;
        double _add35r = 0, _add35i = 0;
        _add35r = j + _c34r; _add35i = 0 + _c34i;
        double _cos36r = 0, _cos36i = 0;
        c_cos(_add35r, _add35i, &_cos36r, &_cos36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(_sin33r, _sin33i, _cos36r, _cos36i, &_mul37r, &_mul37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _sub30r + _mul37r; _add38i = _sub30i + _mul37i;
        double angle_r = _add38r, angle_i = _add38i;
        double _cos39r = 0, _cos39i = 0;
        c_cos(angle_r, angle_i, &_cos39r, &_cos39i);
        double _sin40r = 0, _sin40i = 0;
        c_sin(angle_r, angle_i, &_sin40r, &_sin40i);
        double _c41r = 0, _c41i = 0;
        _c41r = 0.0; _c41i = 1.0;
        double _mul42r = 0, _mul42i = 0;
        c_mul(_sin40r, _sin40i, _c41r, _c41i, &_mul42r, &_mul42i);
        double _add43r = 0, _add43i = 0;
        _add43r = _cos39r + _mul42r; _add43i = _cos39i + _mul42i;
        double _mul44r = 0, _mul44i = 0;
        c_mul(mag_r, mag_i, _add43r, _add43i, &_mul44r, &_mul44i);
        { int _idx = j; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul44r; cIm[_idx] = _mul44i; } }
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_214_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _re1r = 0, _re1i = 0;
    _re1r = x1r; _re1i = 0;
    double _im2r = 0, _im2i = 0;
    _im2r = x2i; _im2i = 0;
    double _c3r = 0, _c3i = 0;
    _c3r = 0.0; _c3i = 1.0;
    double _mul4r = 0, _mul4i = 0;
    c_mul(_im2r, _im2i, _c3r, _c3i, &_mul4r, &_mul4i);
    double _add5r = 0, _add5i = 0;
    _add5r = _re1r + _mul4r; _add5i = _re1i + _mul4i;
    { int _idx = 0; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add5r; cIm[_idx] = _add5i; } }
    double _mul6r = 0, _mul6i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul6r, &_mul6i);
    double prev_r = _mul6r, prev_i = _mul6i;
    for (int j = 0; j < 70; j++) {
        double _abs7r = 0, _abs7i = 0;
        _abs7r = c_abs(prev_r, prev_i); _abs7i = 0;
        double _c8r = 0, _c8i = 0;
        _c8r = 1.0; _c8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = _abs7r + _c8r; _add9i = _abs7i + _c8i;
        double _log10r = 0, _log10i = 0;
        c_log(_add9r, _add9i, &_log10r, &_log10i);
        double _re11r = 0, _re11i = 0;
        _re11r = prev_r; _re11i = 0;
        double _c12r = 0, _c12i = 0;
        _c12r = 2.0; _c12i = 0;
        double _pow13r = 0, _pow13i = 0;
        c_mul(_re11r, _re11i, _re11r, _re11i, &_pow13r, &_pow13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _log10r + _pow13r; _add14i = _log10i + _pow13i;
        double _im15r = 0, _im15i = 0;
        _im15r = prev_i; _im15i = 0;
        double _c16r = 0, _c16i = 0;
        _c16r = 2.0; _c16i = 0;
        double _pow17r = 0, _pow17i = 0;
        c_mul(_im15r, _im15i, _im15r, _im15i, &_pow17r, &_pow17i);
        double _sub18r = 0, _sub18i = 0;
        _sub18r = _add14r - _pow17r; _sub18i = _add14i - _pow17i;
        double magnitude_r = _sub18r, magnitude_i = _sub18i;
        double _ang19r = 0, _ang19i = 0;
        _ang19r = c_arg(prev_r, prev_i); _ang19i = 0;
        double _re20r = 0, _re20i = 0;
        _re20r = prev_r; _re20i = 0;
        double _sin21r = 0, _sin21i = 0;
        c_sin(_re20r, _re20i, &_sin21r, &_sin21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _ang19r + _sin21r; _add22i = _ang19i + _sin21i;
        double _im23r = 0, _im23i = 0;
        _im23r = prev_i; _im23i = 0;
        double _cos24r = 0, _cos24i = 0;
        c_cos(_im23r, _im23i, &_cos24r, &_cos24i);
        double _sub25r = 0, _sub25i = 0;
        _sub25r = _add22r - _cos24r; _sub25i = _add22i - _cos24i;
        double angle_r = _sub25r, angle_i = _sub25i;
        double _cos26r = 0, _cos26i = 0;
        c_cos(angle_r, angle_i, &_cos26r, &_cos26i);
        double _sin27r = 0, _sin27i = 0;
        c_sin(angle_r, angle_i, &_sin27r, &_sin27i);
        double _c28r = 0, _c28i = 0;
        _c28r = 0.0; _c28i = 1.0;
        double _mul29r = 0, _mul29i = 0;
        c_mul(_sin27r, _sin27i, _c28r, _c28i, &_mul29r, &_mul29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _cos26r + _mul29r; _add30i = _cos26i + _mul29i;
        double _mul31r = 0, _mul31i = 0;
        c_mul(magnitude_r, magnitude_i, _add30r, _add30i, &_mul31r, &_mul31i);
        { int _idx = (j + 1); if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul31r; cIm[_idx] = _mul31i; } }
        double _mul32r = 0, _mul32i = 0;
        c_mul(prev_r, prev_i, x1r, x1i, &_mul32r, &_mul32i);
        double _c33r = 0, _c33i = 0;
        _c33r = 1.0; _c33i = 0;
        double _add34r = 0, _add34i = 0;
        _add34r = j + _c33r; _add34i = 0 + _c33i;
        double _div35r = 0, _div35i = 0;
        c_div(x2r, x2i, _add34r, _add34i, &_div35r, &_div35i);
        double _sub36r = 0, _sub36i = 0;
        _sub36r = _mul32r - _div35r; _sub36i = _mul32i - _div35i;
        prev_r = _sub36r; prev_i = _sub36i;
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_215_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 0; j < 71; j++) {
        double _re1r = 0, _re1i = 0;
        _re1r = x1r; _re1i = 0;
        double _c2r = 0, _c2i = 0;
        _c2r = 1.0; _c2i = 0;
        double _add3r = 0, _add3i = 0;
        _add3r = j + _c2r; _add3i = 0 + _c2i;
        double _add4r = 0, _add4i = 0;
        _add4r = _re1r + _add3r; _add4i = _re1i + _add3i;
        double _c5r = 0, _c5i = 0;
        _c5r = 1.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = j + _c5r; _add6i = 0 + _c5i;
        double _ang7r = 0, _ang7i = 0;
        _ang7r = c_arg(x1r, x1i); _ang7i = 0;
        double _mul8r = 0, _mul8i = 0;
        c_mul(_add6r, _add6i, _ang7r, _ang7i, &_mul8r, &_mul8i);
        double _sin9r = 0, _sin9i = 0;
        c_sin(_mul8r, _mul8i, &_sin9r, &_sin9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(_add4r, _add4i, _sin9r, _sin9i, &_mul10r, &_mul10i);
        double _conj11r = 0, _conj11i = 0;
        _conj11r = x2r; _conj11i = -(x2i);
        double _im12r = 0, _im12i = 0;
        _im12r = x1i; _im12i = 0;
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = j + _c13r; _add14i = 0 + _c13i;
        double _add15r = 0, _add15i = 0;
        _add15r = _im12r + _add14r; _add15i = _im12i + _add14i;
        double _mul16r = 0, _mul16i = 0;
        c_mul(_conj11r, _conj11i, _add15r, _add15i, &_mul16r, &_mul16i);
        double _c17r = 0, _c17i = 0;
        _c17r = 1.0; _c17i = 0;
        double _add18r = 0, _add18i = 0;
        _add18r = j + _c17r; _add18i = 0 + _c17i;
        double _ang19r = 0, _ang19i = 0;
        _ang19r = c_arg(x2r, x2i); _ang19i = 0;
        double _mul20r = 0, _mul20i = 0;
        c_mul(_add18r, _add18i, _ang19r, _ang19i, &_mul20r, &_mul20i);
        double _cos21r = 0, _cos21i = 0;
        c_cos(_mul20r, _mul20i, &_cos21r, &_cos21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_mul16r, _mul16i, _cos21r, _cos21i, &_mul22r, &_mul22i);
        double _c23r = 0, _c23i = 0;
        _c23r = 0.0; _c23i = 1.0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(_mul22r, _mul22i, _c23r, _c23i, &_mul24r, &_mul24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _mul10r + _mul24r; _add25i = _mul10i + _mul24i;
        { int _idx = j; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add25r; cIm[_idx] = _add25i; } }
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_216_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 0; j < 71; j++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 1.0; _c1i = 0;
        double _add2r = 0, _add2i = 0;
        _add2r = j + _c1r; _add2i = 0 + _c1i;
        double _c3r = 0, _c3i = 0;
        _c3r = 5.0; _c3i = 0;
        double _mul4r = 0, _mul4i = 0;
        c_mul(_add2r, _add2i, _c3r, _c3i, &_mul4r, &_mul4i);
        double _c5r = 0, _c5i = 0;
        _c5r = 20.0; _c5i = 0;
        double _mod6r = 0, _mod6i = 0;
        _mod6r = fmod(_mul4r, _c5r); _mod6i = 0;
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _mod6r + _c7r; _add8i = _mod6i + _c7i;
        double k_r = _add8r, k_i = _add8i;
        double _c9r = 0, _c9i = 0;
        _c9r = 6.0; _c9i = 0;
        double _fdiv10r = 0, _fdiv10i = 0;
        c_div(j, 0, _c9r, _c9i, &_fdiv10r, &_fdiv10i);
        _fdiv10r = floor(_fdiv10r); _fdiv10i = 0;
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = _fdiv10r + _c11r; _add12i = _fdiv10i + _c11i;
        double r_r = _add12r, r_i = _add12i;
        double _re13r = 0, _re13i = 0;
        _re13r = x1r; _re13i = 0;
        double _pow14r = 0, _pow14i = 0;
        c_powc(_re13r, _re13i, k_r, k_i, &_pow14r, &_pow14i);
        double _im15r = 0, _im15i = 0;
        _im15r = x2i; _im15i = 0;
        double _pow16r = 0, _pow16i = 0;
        c_powc(_im15r, _im15i, r_r, r_i, &_pow16r, &_pow16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _pow14r + _pow16r; _add17i = _pow14i + _pow16i;
        double _c18r = 0, _c18i = 0;
        _c18r = 1.0; _c18i = 0;
        double _add19r = 0, _add19i = 0;
        _add19r = j + _c18r; _add19i = 0 + _c18i;
        double _ang20r = 0, _ang20i = 0;
        _ang20r = c_arg(x1r, x1i); _ang20i = 0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(_add19r, _add19i, _ang20r, _ang20i, &_mul21r, &_mul21i);
        double _cos22r = 0, _cos22i = 0;
        c_cos(_mul21r, _mul21i, &_cos22r, &_cos22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_add17r, _add17i, _cos22r, _cos22i, &_mul23r, &_mul23i);
        double _conj24r = 0, _conj24i = 0;
        _conj24r = x2r; _conj24i = -(x2i);
        double _c25r = 0, _c25i = 0;
        _c25r = 1.0; _c25i = 0;
        double _add26r = 0, _add26i = 0;
        _add26r = j + _c25r; _add26i = 0 + _c25i;
        double _ang27r = 0, _ang27i = 0;
        _ang27r = c_arg(x2r, x2i); _ang27i = 0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(_add26r, _add26i, _ang27r, _ang27i, &_mul28r, &_mul28i);
        double _sin29r = 0, _sin29i = 0;
        c_sin(_mul28r, _mul28i, &_sin29r, &_sin29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_conj24r, _conj24i, _sin29r, _sin29i, &_mul30r, &_mul30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _mul23r + _mul30r; _add31i = _mul23i + _mul30i;
        double _mul32r = 0, _mul32i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul32r, &_mul32i);
        double _re33r = 0, _re33i = 0;
        _re33r = _mul32r; _re33i = 0;
        double _c34r = 0, _c34i = 0;
        _c34r = 1.0; _c34i = 0;
        double _add35r = 0, _add35i = 0;
        _add35r = j + _c34r; _add35i = 0 + _c34i;
        double _cos36r = 0, _cos36i = 0;
        c_cos(_add35r, _add35i, &_cos36r, &_cos36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(_re33r, _re33i, _cos36r, _cos36i, &_mul37r, &_mul37i);
        double _sub38r = 0, _sub38i = 0;
        _sub38r = _add31r - _mul37r; _sub38i = _add31i - _mul37i;
        { int _idx = j; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub38r; cIm[_idx] = _sub38i; } }
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_217_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 0; j < 71; j++) {
        double _ang1r = 0, _ang1i = 0;
        _ang1r = c_arg(x1r, x1i); _ang1i = 0;
        double _c2r = 0, _c2i = 0;
        _c2r = 1.0; _c2i = 0;
        double _add3r = 0, _add3i = 0;
        _add3r = j + _c2r; _add3i = 0 + _c2i;
        double _mul4r = 0, _mul4i = 0;
        c_mul(_ang1r, _ang1i, _add3r, _add3i, &_mul4r, &_mul4i);
        double _ang5r = 0, _ang5i = 0;
        _ang5r = c_arg(x2r, x2i); _ang5i = 0;
        double _c6r = 0, _c6i = 0;
        _c6r = 71.0; _c6i = 0;
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = j + _c7r; _add8i = 0 + _c7i;
        double _sub9r = 0, _sub9i = 0;
        _sub9r = _c6r - _add8r; _sub9i = _c6i - _add8i;
        double _mul10r = 0, _mul10i = 0;
        c_mul(_ang5r, _ang5i, _sub9r, _sub9i, &_mul10r, &_mul10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _mul4r + _mul10r; _add11i = _mul4i + _mul10i;
        double angle_r = _add11r, angle_i = _add11i;
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(x1r, x1i); _abs12i = 0;
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = j + _c13r; _add14i = 0 + _c13i;
        double _pow15r = 0, _pow15i = 0;
        c_powc(_abs12r, _abs12i, _add14r, _add14i, &_pow15r, &_pow15i);
        double _abs16r = 0, _abs16i = 0;
        _abs16r = c_abs(x2r, x2i); _abs16i = 0;
        double _c17r = 0, _c17i = 0;
        _c17r = 71.0; _c17i = 0;
        double _c18r = 0, _c18i = 0;
        _c18r = 1.0; _c18i = 0;
        double _add19r = 0, _add19i = 0;
        _add19r = j + _c18r; _add19i = 0 + _c18i;
        double _sub20r = 0, _sub20i = 0;
        _sub20r = _c17r - _add19r; _sub20i = _c17i - _add19i;
        double _pow21r = 0, _pow21i = 0;
        c_powc(_abs16r, _abs16i, _sub20r, _sub20i, &_pow21r, &_pow21i);
        double _mul22r = 0, _mul22i = 0;
        c_mul(_pow15r, _pow15i, _pow21r, _pow21i, &_mul22r, &_mul22i);
        double _abs23r = 0, _abs23i = 0;
        _abs23r = c_abs(x1r, x1i); _abs23i = 0;
        double _c24r = 0, _c24i = 0;
        _c24r = 1.0; _c24i = 0;
        double _add25r = 0, _add25i = 0;
        _add25r = _abs23r + _c24r; _add25i = _abs23i + _c24i;
        double _log26r = 0, _log26i = 0;
        c_log(_add25r, _add25i, &_log26r, &_log26i);
        double _c27r = 0, _c27i = 0;
        _c27r = 1.0; _c27i = 0;
        double _add28r = 0, _add28i = 0;
        _add28r = j + _c27r; _add28i = 0 + _c27i;
        double _sin29r = 0, _sin29i = 0;
        c_sin(_add28r, _add28i, &_sin29r, &_sin29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_log26r, _log26i, _sin29r, _sin29i, &_mul30r, &_mul30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _mul22r + _mul30r; _add31i = _mul22i + _mul30i;
        double _c32r = 0, _c32i = 0;
        _c32r = 1.0; _c32i = 0;
        double _add33r = 0, _add33i = 0;
        _add33r = j + _c32r; _add33i = 0 + _c32i;
        double _cos34r = 0, _cos34i = 0;
        c_cos(_add33r, _add33i, &_cos34r, &_cos34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _add31r + _cos34r; _add35i = _add31i + _cos34i;
        double magnitude_r = _add35r, magnitude_i = _add35i;
        double _cos36r = 0, _cos36i = 0;
        c_cos(angle_r, angle_i, &_cos36r, &_cos36i);
        double _sin37r = 0, _sin37i = 0;
        c_sin(angle_r, angle_i, &_sin37r, &_sin37i);
        double _c38r = 0, _c38i = 0;
        _c38r = 0.0; _c38i = 1.0;
        double _mul39r = 0, _mul39i = 0;
        c_mul(_sin37r, _sin37i, _c38r, _c38i, &_mul39r, &_mul39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _cos36r + _mul39r; _add40i = _cos36i + _mul39i;
        double _mul41r = 0, _mul41i = 0;
        c_mul(magnitude_r, magnitude_i, _add40r, _add40i, &_mul41r, &_mul41i);
        { int _idx = j; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul41r; cIm[_idx] = _mul41i; } }
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_218_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 0; k < 71; k++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 1.0; _c1i = 0;
        double _add2r = 0, _add2i = 0;
        _add2r = k + _c1r; _add2i = 0 + _c1i;
        double _abs3r = 0, _abs3i = 0;
        _abs3r = c_abs(x1r, x1i); _abs3i = 0;
        double _mul4r = 0, _mul4i = 0;
        c_mul(_add2r, _add2i, _abs3r, _abs3i, &_mul4r, &_mul4i);
        double _sin5r = 0, _sin5i = 0;
        c_sin(_mul4r, _mul4i, &_sin5r, &_sin5i);
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = k + _c6r; _add7i = 0 + _c6i;
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(x2r, x2i); _abs8i = 0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(_add7r, _add7i, _abs8r, _abs8i, &_mul9r, &_mul9i);
        double _cos10r = 0, _cos10i = 0;
        c_cos(_mul9r, _mul9i, &_cos10r, &_cos10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _sin5r + _cos10r; _add11i = _sin5i + _cos10i;
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(x1r, x1i); _abs12i = 0;
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = k + _c13r; _add14i = 0 + _c13i;
        double _add15r = 0, _add15i = 0;
        _add15r = _abs12r + _add14r; _add15i = _abs12i + _add14i;
        double _log16r = 0, _log16i = 0;
        c_log(_add15r, _add15i, &_log16r, &_log16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _add11r + _log16r; _add17i = _add11i + _log16i;
        double mag_r = _add17r, mag_i = _add17i;
        double _ang18r = 0, _ang18i = 0;
        _ang18r = c_arg(x1r, x1i); _ang18i = 0;
        double _c19r = 0, _c19i = 0;
        _c19r = 1.0; _c19i = 0;
        double _add20r = 0, _add20i = 0;
        _add20r = k + _c19r; _add20i = 0 + _c19i;
        double _mul21r = 0, _mul21i = 0;
        c_mul(_ang18r, _ang18i, _add20r, _add20i, &_mul21r, &_mul21i);
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x2r, x2i); _ang22i = 0;
        double _c23r = 0, _c23i = 0;
        _c23r = 71.0; _c23i = 0;
        double _c24r = 0, _c24i = 0;
        _c24r = 1.0; _c24i = 0;
        double _add25r = 0, _add25i = 0;
        _add25r = k + _c24r; _add25i = 0 + _c24i;
        double _sub26r = 0, _sub26i = 0;
        _sub26r = _c23r - _add25r; _sub26i = _c23i - _add25i;
        double _mul27r = 0, _mul27i = 0;
        c_mul(_ang22r, _ang22i, _sub26r, _sub26i, &_mul27r, &_mul27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _mul21r + _mul27r; _add28i = _mul21i + _mul27i;
        double angle_r = _add28r, angle_i = _add28i;
        double _cos29r = 0, _cos29i = 0;
        c_cos(angle_r, angle_i, &_cos29r, &_cos29i);
        double _sin30r = 0, _sin30i = 0;
        c_sin(angle_r, angle_i, &_sin30r, &_sin30i);
        double _c31r = 0, _c31i = 0;
        _c31r = 0.0; _c31i = 1.0;
        double _mul32r = 0, _mul32i = 0;
        c_mul(_sin30r, _sin30i, _c31r, _c31i, &_mul32r, &_mul32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _cos29r + _mul32r; _add33i = _cos29i + _mul32i;
        double _mul34r = 0, _mul34i = 0;
        c_mul(mag_r, mag_i, _add33r, _add33i, &_mul34r, &_mul34i);
        { int _idx = k; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul34r; cIm[_idx] = _mul34i; } }
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_219_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 0; k < 71; k++) {
        double _ang1r = 0, _ang1i = 0;
        _ang1r = c_arg(x1r, x1i); _ang1i = 0;
        double _c2r = 0, _c2i = 0;
        _c2r = 1.0; _c2i = 0;
        double _add3r = 0, _add3i = 0;
        _add3r = k + _c2r; _add3i = 0 + _c2i;
        double _mul4r = 0, _mul4i = 0;
        c_mul(_ang1r, _ang1i, _add3r, _add3i, &_mul4r, &_mul4i);
        double _ang5r = 0, _ang5i = 0;
        _ang5r = c_arg(x2r, x2i); _ang5i = 0;
        double _c6r = 0, _c6i = 0;
        _c6r = 71.0; _c6i = 0;
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = k + _c7r; _add8i = 0 + _c7i;
        double _sub9r = 0, _sub9i = 0;
        _sub9r = _c6r - _add8r; _sub9i = _c6i - _add8i;
        double _mul10r = 0, _mul10i = 0;
        c_mul(_ang5r, _ang5i, _sub9r, _sub9i, &_mul10r, &_mul10i);
        double _sub11r = 0, _sub11i = 0;
        _sub11r = _mul4r - _mul10r; _sub11i = _mul4i - _mul10i;
        double angle_r = _sub11r, angle_i = _sub11i;
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(x1r, x1i); _abs12i = 0;
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = k + _c13r; _add14i = 0 + _c13i;
        double _pow15r = 0, _pow15i = 0;
        c_powc(_abs12r, _abs12i, _add14r, _add14i, &_pow15r, &_pow15i);
        double _abs16r = 0, _abs16i = 0;
        _abs16r = c_abs(x2r, x2i); _abs16i = 0;
        double _c17r = 0, _c17i = 0;
        _c17r = 1.0; _c17i = 0;
        double _add18r = 0, _add18i = 0;
        _add18r = _abs16r + _c17r; _add18i = _abs16i + _c17i;
        double _log19r = 0, _log19i = 0;
        c_log(_add18r, _add18i, &_log19r, &_log19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 1.0; _c20i = 0;
        double _add21r = 0, _add21i = 0;
        _add21r = k + _c20r; _add21i = 0 + _c20i;
        double _pow22r = 0, _pow22i = 0;
        c_powc(_log19r, _log19i, _add21r, _add21i, &_pow22r, &_pow22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _pow15r + _pow22r; _add23i = _pow15i + _pow22i;
        double magnitude_r = _add23r, magnitude_i = _add23i;
        double _cos24r = 0, _cos24i = 0;
        c_cos(angle_r, angle_i, &_cos24r, &_cos24i);
        double _sin25r = 0, _sin25i = 0;
        c_sin(angle_r, angle_i, &_sin25r, &_sin25i);
        double _c26r = 0, _c26i = 0;
        _c26r = 0.0; _c26i = 1.0;
        double _mul27r = 0, _mul27i = 0;
        c_mul(_sin25r, _sin25i, _c26r, _c26i, &_mul27r, &_mul27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _cos24r + _mul27r; _add28i = _cos24i + _mul27i;
        double _mul29r = 0, _mul29i = 0;
        c_mul(magnitude_r, magnitude_i, _add28r, _add28i, &_mul29r, &_mul29i);
        { int _idx = k; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul29r; cIm[_idx] = _mul29i; } }
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_220_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 0; j < 71; j++) {
        double _re1r = 0, _re1i = 0;
        _re1r = x1r; _re1i = 0;
        double _c2r = 0, _c2i = 0;
        _c2r = 1.0; _c2i = 0;
        double _add3r = 0, _add3i = 0;
        _add3r = j + _c2r; _add3i = 0 + _c2i;
        double _mul4r = 0, _mul4i = 0;
        c_mul(_re1r, _re1i, _add3r, _add3i, &_mul4r, &_mul4i);
        double _im5r = 0, _im5i = 0;
        _im5r = x2i; _im5i = 0;
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = j + _c6r; _add7i = 0 + _c6i;
        double _c8r = 0, _c8i = 0;
        _c8r = 2.0; _c8i = 0;
        double _pow9r = 0, _pow9i = 0;
        c_mul(_add7r, _add7i, _add7r, _add7i, &_pow9r, &_pow9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(_im5r, _im5i, _pow9r, _pow9i, &_mul10r, &_mul10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _mul4r + _mul10r; _add11i = _mul4i + _mul10i;
        double _ang12r = 0, _ang12i = 0;
        _ang12r = c_arg(x1r, x1i); _ang12i = 0;
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = j + _c13r; _add14i = 0 + _c13i;
        double _mul15r = 0, _mul15i = 0;
        c_mul(_ang12r, _ang12i, _add14r, _add14i, &_mul15r, &_mul15i);
        double _sin16r = 0, _sin16i = 0;
        c_sin(_mul15r, _mul15i, &_sin16r, &_sin16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_add11r, _add11i, _sin16r, _sin16i, &_mul17r, &_mul17i);
        double _ang18r = 0, _ang18i = 0;
        _ang18r = c_arg(x2r, x2i); _ang18i = 0;
        double _c19r = 0, _c19i = 0;
        _c19r = 1.0; _c19i = 0;
        double _add20r = 0, _add20i = 0;
        _add20r = j + _c19r; _add20i = 0 + _c19i;
        double _mul21r = 0, _mul21i = 0;
        c_mul(_ang18r, _ang18i, _add20r, _add20i, &_mul21r, &_mul21i);
        double _cos22r = 0, _cos22i = 0;
        c_cos(_mul21r, _mul21i, &_cos22r, &_cos22i);
        double _abs23r = 0, _abs23i = 0;
        _abs23r = c_abs(x1r, x1i); _abs23i = 0;
        double _abs24r = 0, _abs24i = 0;
        _abs24r = c_abs(x2r, x2i); _abs24i = 0;
        double _c25r = 0, _c25i = 0;
        _c25r = 1.0; _c25i = 0;
        double _add26r = 0, _add26i = 0;
        _add26r = j + _c25r; _add26i = 0 + _c25i;
        double _mul27r = 0, _mul27i = 0;
        c_mul(_abs24r, _abs24i, _add26r, _add26i, &_mul27r, &_mul27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _abs23r + _mul27r; _add28i = _abs23i + _mul27i;
        double _log29r = 0, _log29i = 0;
        c_log(_add28r, _add28i, &_log29r, &_log29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_cos22r, _cos22i, _log29r, _log29i, &_mul30r, &_mul30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _mul17r + _mul30r; _add31i = _mul17i + _mul30i;
        double _conj32r = 0, _conj32i = 0;
        _conj32r = x1r; _conj32i = -(x1i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_conj32r, _conj32i, x2r, x2i, &_mul33r, &_mul33i);
        double _re34r = 0, _re34i = 0;
        _re34r = _mul33r; _re34i = 0;
        double _c35r = 0, _c35i = 0;
        _c35r = 1.0; _c35i = 0;
        double _add36r = 0, _add36i = 0;
        _add36r = j + _c35r; _add36i = 0 + _c35i;
        double _pow37r = 0, _pow37i = 0;
        c_powc(_re34r, _re34i, _add36r, _add36i, &_pow37r, &_pow37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _add31r + _pow37r; _add38i = _add31i + _pow37i;
        double _conj39r = 0, _conj39i = 0;
        _conj39r = x2r; _conj39i = -(x2i);
        double _mul40r = 0, _mul40i = 0;
        c_mul(x1r, x1i, _conj39r, _conj39i, &_mul40r, &_mul40i);
        double _im41r = 0, _im41i = 0;
        _im41r = _mul40i; _im41i = 0;
        double _c42r = 0, _c42i = 0;
        _c42r = 1.0; _c42i = 0;
        double _add43r = 0, _add43i = 0;
        _add43r = j + _c42r; _add43i = 0 + _c42i;
        double _pow44r = 0, _pow44i = 0;
        c_powc(_im41r, _im41i, _add43r, _add43i, &_pow44r, &_pow44i);
        double _sub45r = 0, _sub45i = 0;
        _sub45r = _add38r - _pow44r; _sub45i = _add38i - _pow44i;
        { int _idx = j; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub45r; cIm[_idx] = _sub45i; } }
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_221_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 0; j < 71; j++) {
        double _re1r = 0, _re1i = 0;
        _re1r = x1r; _re1i = 0;
        double _c2r = 0, _c2i = 0;
        _c2r = 1.0; _c2i = 0;
        double _add3r = 0, _add3i = 0;
        _add3r = j + _c2r; _add3i = 0 + _c2i;
        double _pow4r = 0, _pow4i = 0;
        c_powc(_re1r, _re1i, _add3r, _add3i, &_pow4r, &_pow4i);
        double _ang5r = 0, _ang5i = 0;
        _ang5r = c_arg(x2r, x2i); _ang5i = 0;
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = j + _c6r; _add7i = 0 + _c6i;
        double _add8r = 0, _add8i = 0;
        _add8r = _ang5r + _add7r; _add8i = _ang5i + _add7i;
        double _cos9r = 0, _cos9i = 0;
        c_cos(_add8r, _add8i, &_cos9r, &_cos9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(_pow4r, _pow4i, _cos9r, _cos9i, &_mul10r, &_mul10i);
        double _im11r = 0, _im11i = 0;
        _im11r = x2i; _im11i = 0;
        double _c12r = 0, _c12i = 0;
        _c12r = 1.0; _c12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = j + _c12r; _add13i = 0 + _c12i;
        double _pow14r = 0, _pow14i = 0;
        c_powc(_im11r, _im11i, _add13r, _add13i, &_pow14r, &_pow14i);
        double _ang15r = 0, _ang15i = 0;
        _ang15r = c_arg(x1r, x1i); _ang15i = 0;
        double _c16r = 0, _c16i = 0;
        _c16r = 1.0; _c16i = 0;
        double _add17r = 0, _add17i = 0;
        _add17r = j + _c16r; _add17i = 0 + _c16i;
        double _mul18r = 0, _mul18i = 0;
        c_mul(_ang15r, _ang15i, _add17r, _add17i, &_mul18r, &_mul18i);
        double _sin19r = 0, _sin19i = 0;
        c_sin(_mul18r, _mul18i, &_sin19r, &_sin19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_pow14r, _pow14i, _sin19r, _sin19i, &_mul20r, &_mul20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _mul10r + _mul20r; _add21i = _mul10i + _mul20i;
        double _abs22r = 0, _abs22i = 0;
        _abs22r = c_abs(x1r, x1i); _abs22i = 0;
        double _c23r = 0, _c23i = 0;
        _c23r = 1.0; _c23i = 0;
        double _add24r = 0, _add24i = 0;
        _add24r = j + _c23r; _add24i = 0 + _c23i;
        double _add25r = 0, _add25i = 0;
        _add25r = _abs22r + _add24r; _add25i = _abs22i + _add24i;
        double _log26r = 0, _log26i = 0;
        c_log(_add25r, _add25i, &_log26r, &_log26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _add21r + _log26r; _add27i = _add21i + _log26i;
        double _abs28r = 0, _abs28i = 0;
        _abs28r = c_abs(x2r, x2i); _abs28i = 0;
        double _c29r = 0, _c29i = 0;
        _c29r = 1.0; _c29i = 0;
        double _add30r = 0, _add30i = 0;
        _add30r = _abs28r + _c29r; _add30i = _abs28i + _c29i;
        double _log31r = 0, _log31i = 0;
        c_log(_add30r, _add30i, &_log31r, &_log31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _add27r + _log31r; _add32i = _add27i + _log31i;
        double _conj33r = 0, _conj33i = 0;
        _conj33r = x1r; _conj33i = -(x1i);
        double _c34r = 0, _c34i = 0;
        _c34r = 1.0; _c34i = 0;
        double _add35r = 0, _add35i = 0;
        _add35r = j + _c34r; _add35i = 0 + _c34i;
        double _mul36r = 0, _mul36i = 0;
        c_mul(_conj33r, _conj33i, _add35r, _add35i, &_mul36r, &_mul36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _add32r + _mul36r; _add37i = _add32i + _mul36i;
        double _conj38r = 0, _conj38i = 0;
        _conj38r = x2r; _conj38i = -(x2i);
        double _c39r = 0, _c39i = 0;
        _c39r = 1.0; _c39i = 0;
        double _add40r = 0, _add40i = 0;
        _add40r = j + _c39r; _add40i = 0 + _c39i;
        double _pow41r = 0, _pow41i = 0;
        c_powc(_conj38r, _conj38i, _add40r, _add40i, &_pow41r, &_pow41i);
        double _sub42r = 0, _sub42i = 0;
        _sub42r = _add37r - _pow41r; _sub42i = _add37i - _pow41i;
        { int _idx = j; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _sub42r; cIm[_idx] = _sub42i; } }
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_222_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _add1r = 0, _add1i = 0;
    _add1r = x1r + x2r; _add1i = x1i + x2i;
    double prev_r = _add1r, prev_i = _add1i;
    for (int j = 0; j < 71; j++) {
        double _abs2r = 0, _abs2i = 0;
        _abs2r = c_abs(prev_r, prev_i); _abs2i = 0;
        double _abs3r = 0, _abs3i = 0;
        _abs3r = c_abs(prev_r, prev_i); _abs3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 1.0; _c4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = _abs3r + _c4r; _add5i = _abs3i + _c4i;
        double _log6r = 0, _log6i = 0;
        c_log(_add5r, _add5i, &_log6r, &_log6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(_abs2r, _abs2i, _log6r, _log6i, &_mul7r, &_mul7i);
        double magnitude_r = _mul7r, magnitude_i = _mul7i;
        double _ang8r = 0, _ang8i = 0;
        _ang8r = c_arg(prev_r, prev_i); _ang8i = 0;
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = j + _c9r; _add10i = 0 + _c9i;
        double _sin11r = 0, _sin11i = 0;
        c_sin(_add10r, _add10i, &_sin11r, &_sin11i);
        double _c12r = 0, _c12i = 0;
        _c12r = 1.0; _c12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = j + _c12r; _add13i = 0 + _c12i;
        double _cos14r = 0, _cos14i = 0;
        c_cos(_add13r, _add13i, &_cos14r, &_cos14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_sin11r, _sin11i, _cos14r, _cos14i, &_mul15r, &_mul15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _ang8r + _mul15r; _add16i = _ang8i + _mul15i;
        double angle_r = _add16r, angle_i = _add16i;
        double _cos17r = 0, _cos17i = 0;
        c_cos(angle_r, angle_i, &_cos17r, &_cos17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 0.0; _c18i = 1.0;
        double _sin19r = 0, _sin19i = 0;
        c_sin(angle_r, angle_i, &_sin19r, &_sin19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_c18r, _c18i, _sin19r, _sin19i, &_mul20r, &_mul20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _cos17r + _mul20r; _add21i = _cos17i + _mul20i;
        double _mul22r = 0, _mul22i = 0;
        c_mul(magnitude_r, magnitude_i, _add21r, _add21i, &_mul22r, &_mul22i);
        { int _idx = j; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _mul22r; cIm[_idx] = _mul22i; } }
        double _mul23r = 0, _mul23i = 0;
        c_mul(prev_r, prev_i, x1r, x1i, &_mul23r, &_mul23i);
        double _c24r = 0, _c24i = 0;
        _c24r = 1.0; _c24i = 0;
        double _add25r = 0, _add25i = 0;
        _add25r = j + _c24r; _add25i = 0 + _c24i;
        double _div26r = 0, _div26i = 0;
        c_div(x2r, x2i, _add25r, _add25i, &_div26r, &_div26i);
        double _sub27r = 0, _sub27i = 0;
        _sub27r = _mul23r - _div26r; _sub27i = _mul23i - _div26i;
        prev_r = _sub27r; prev_i = _sub27i;
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_223_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 71;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 71; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 0; j < 71; j++) {
        double _abs1r = 0, _abs1i = 0;
        _abs1r = c_abs(x1r, x1i); _abs1i = 0;
        double _c2r = 0, _c2i = 0;
        _c2r = 1.0; _c2i = 0;
        double _add3r = 0, _add3i = 0;
        _add3r = j + _c2r; _add3i = 0 + _c2i;
        double _mul4r = 0, _mul4i = 0;
        c_mul(_abs1r, _abs1i, _add3r, _add3i, &_mul4r, &_mul4i);
        double _c5r = 0, _c5i = 0;
        _c5r = 1.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = _mul4r + _c5r; _add6i = _mul4i + _c5i;
        double _log7r = 0, _log7i = 0;
        c_log(_add6r, _add6i, &_log7r, &_log7i);
        double _c8r = 0, _c8i = 0;
        _c8r = 1.0; _c8i = 0;
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = j + _c9r; _add10i = 0 + _c9i;
        double _ang11r = 0, _ang11i = 0;
        _ang11r = c_arg(x2r, x2i); _ang11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(_add10r, _add10i, _ang11r, _ang11i, &_mul12r, &_mul12i);
        double _sin13r = 0, _sin13i = 0;
        c_sin(_mul12r, _mul12i, &_sin13r, &_sin13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _c8r + _sin13r; _add14i = _c8i + _sin13i;
        double _mul15r = 0, _mul15i = 0;
        c_mul(_log7r, _log7i, _add14r, _add14i, &_mul15r, &_mul15i);
        double mag_r = _mul15r, mag_i = _mul15i;
        double _ang16r = 0, _ang16i = 0;
        _ang16r = c_arg(x1r, x1i); _ang16i = 0;
        double _c17r = 0, _c17i = 0;
        _c17r = 1.0; _c17i = 0;
        double _add18r = 0, _add18i = 0;
        _add18r = j + _c17r; _add18i = 0 + _c17i;
        double _sqrt19r = 0, _sqrt19i = 0;
        c_powr(_add18r, _add18i, 0.5, &_sqrt19r, &_sqrt19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_ang16r, _ang16i, _sqrt19r, _sqrt19i, &_mul20r, &_mul20i);
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
        double _add26r = 0, _add26i = 0;
        _add26r = _mul20r + _cos25r; _add26i = _mul20i + _cos25i;
        double ang_r = _add26r, ang_i = _add26i;
        double _cos27r = 0, _cos27i = 0;
        c_cos(ang_r, ang_i, &_cos27r, &_cos27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(mag_r, mag_i, _cos27r, _cos27i, &_mul28r, &_mul28i);
        double _sin29r = 0, _sin29i = 0;
        c_sin(ang_r, ang_i, &_sin29r, &_sin29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(mag_r, mag_i, _sin29r, _sin29i, &_mul30r, &_mul30i);
        double _c31r = 0, _c31i = 0;
        _c31r = 0.0; _c31i = 1.0;
        double _mul32r = 0, _mul32i = 0;
        c_mul(_mul30r, _mul30i, _c31r, _c31i, &_mul32r, &_mul32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _mul28r + _mul32r; _add33i = _mul28i + _mul32i;
        { int _idx = j; if (_idx >= 0 && _idx < 71) { cRe[_idx] = _add33r; cIm[_idx] = _add33i; } }
    }
    for (int _i = 0; _i < 71; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_224_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 0; j < 35; j++) {
        double k = 0;
        double _c1r = 0, _c1i = 0;
        _c1r = 1.0; _c1i = 0;
        double _add2r = 0, _add2i = 0;
        _add2r = j + _c1r; _add2i = 0 + _c1i;
        double _c3r = 0, _c3i = 0;
        _c3r = 4.0; _c3i = 0;
        double _mod4r = 0, _mod4i = 0;
        _mod4r = fmod(_add2r, _c3r); _mod4i = 0;
        double _c5r = 0, _c5i = 0;
        _c5r = 1.0; _c5i = 0;
        if (_mod4r == _c5r) {
            double _c6r = 0, _c6i = 0;
            _c6r = 3.0; _c6i = 0;
            double _add7r = 0, _add7i = 0;
            _add7r = j + _c6r; _add7i = 0 + _c6i;
            k = _add7r;
            double _re8r = 0, _re8i = 0;
            _re8r = x1r; _re8i = 0;
            double _pow9r = 0, _pow9i = 0;
            c_powr(_re8r, _re8i, k, &_pow9r, &_pow9i);
            double _im10r = 0, _im10i = 0;
            _im10r = x2i; _im10i = 0;
            double _pow11r = 0, _pow11i = 0;
            c_powr(_im10r, _im10i, k, &_pow11r, &_pow11i);
            double _ang12r = 0, _ang12i = 0;
            _ang12r = c_arg(x1r, x1i); _ang12i = 0;
            double _mul13r = 0, _mul13i = 0;
            c_mul(_ang12r, _ang12i, k, 0, &_mul13r, &_mul13i);
            double _sin14r = 0, _sin14i = 0;
            c_sin(_mul13r, _mul13i, &_sin14r, &_sin14i);
            double _mul15r = 0, _mul15i = 0;
            c_mul(_pow11r, _pow11i, _sin14r, _sin14i, &_mul15r, &_mul15i);
            double _add16r = 0, _add16i = 0;
            _add16r = _pow9r + _mul15r; _add16i = _pow9i + _mul15i;
            { int _idx = j; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add16r; cIm[_idx] = _add16i; } }
        } else {
            double _c17r = 0, _c17i = 0;
            _c17r = 1.0; _c17i = 0;
            double _add18r = 0, _add18i = 0;
            _add18r = j + _c17r; _add18i = 0 + _c17i;
            double _c19r = 0, _c19i = 0;
            _c19r = 4.0; _c19i = 0;
            double _mod20r = 0, _mod20i = 0;
            _mod20r = fmod(_add18r, _c19r); _mod20i = 0;
            double _c21r = 0, _c21i = 0;
            _c21r = 2.0; _c21i = 0;
            if (_mod20r == _c21r) {
                double _c22r = 0, _c22i = 0;
                _c22r = 4.0; _c22i = 0;
                double _add23r = 0, _add23i = 0;
                _add23r = j + _c22r; _add23i = 0 + _c22i;
                k = _add23r;
                double _add24r = 0, _add24i = 0;
                _add24r = x1r + x2r; _add24i = x1i + x2i;
                double _abs25r = 0, _abs25i = 0;
                _abs25r = c_abs(_add24r, _add24i); _abs25i = 0;
                double _pow26r = 0, _pow26i = 0;
                c_powr(_abs25r, _abs25i, k, &_pow26r, &_pow26i);
                double _ang27r = 0, _ang27i = 0;
                _ang27r = c_arg(x2r, x2i); _ang27i = 0;
                double _mul28r = 0, _mul28i = 0;
                c_mul(_ang27r, _ang27i, k, 0, &_mul28r, &_mul28i);
                double _cos29r = 0, _cos29i = 0;
                c_cos(_mul28r, _mul28i, &_cos29r, &_cos29i);
                double _mul30r = 0, _mul30i = 0;
                c_mul(_pow26r, _pow26i, _cos29r, _cos29i, &_mul30r, &_mul30i);
                double _abs31r = 0, _abs31i = 0;
                _abs31r = c_abs(x1r, x1i); _abs31i = 0;
                double _c32r = 0, _c32i = 0;
                _c32r = 1.0; _c32i = 0;
                double _add33r = 0, _add33i = 0;
                _add33r = _abs31r + _c32r; _add33i = _abs31i + _c32i;
                double _log34r = 0, _log34i = 0;
                c_log(_add33r, _add33i, &_log34r, &_log34i);
                double _add35r = 0, _add35i = 0;
                _add35r = _mul30r + _log34r; _add35i = _mul30i + _log34i;
                { int _idx = j; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add35r; cIm[_idx] = _add35i; } }
            } else {
                double _c36r = 0, _c36i = 0;
                _c36r = 1.0; _c36i = 0;
                double _add37r = 0, _add37i = 0;
                _add37r = j + _c36r; _add37i = 0 + _c36i;
                double _c38r = 0, _c38i = 0;
                _c38r = 4.0; _c38i = 0;
                double _mod39r = 0, _mod39i = 0;
                _mod39r = fmod(_add37r, _c38r); _mod39i = 0;
                double _c40r = 0, _c40i = 0;
                _c40r = 3.0; _c40i = 0;
                if (_mod39r == _c40r) {
                    double _c41r = 0, _c41i = 0;
                    _c41r = 2.0; _c41i = 0;
                    double _add42r = 0, _add42i = 0;
                    _add42r = j + _c41r; _add42i = 0 + _c41i;
                    k = _add42r;
                    double _conj43r = 0, _conj43i = 0;
                    _conj43r = x1r; _conj43i = -(x1i);
                    double _pow44r = 0, _pow44i = 0;
                    c_powr(_conj43r, _conj43i, k, &_pow44r, &_pow44i);
                    double _conj45r = 0, _conj45i = 0;
                    _conj45r = x2r; _conj45i = -(x2i);
                    double _pow46r = 0, _pow46i = 0;
                    c_powr(_conj45r, _conj45i, k, &_pow46r, &_pow46i);
                    double _sub47r = 0, _sub47i = 0;
                    _sub47r = _pow44r - _pow46r; _sub47i = _pow44i - _pow46i;
                    double _mul48r = 0, _mul48i = 0;
                    c_mul(x1r, x1i, k, 0, &_mul48r, &_mul48i);
                    double _sin49r = 0, _sin49i = 0;
                    c_sin(_mul48r, _mul48i, &_sin49r, &_sin49i);
                    double _add50r = 0, _add50i = 0;
                    _add50r = _sub47r + _sin49r; _add50i = _sub47i + _sin49i;
                    double _mul51r = 0, _mul51i = 0;
                    c_mul(x2r, x2i, k, 0, &_mul51r, &_mul51i);
                    double _cos52r = 0, _cos52i = 0;
                    c_cos(_mul51r, _mul51i, &_cos52r, &_cos52i);
                    double _sub53r = 0, _sub53i = 0;
                    _sub53r = _add50r - _cos52r; _sub53i = _add50i - _cos52i;
                    { int _idx = j; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub53r; cIm[_idx] = _sub53i; } }
                } else {
                    double _c54r = 0, _c54i = 0;
                    _c54r = 1.0; _c54i = 0;
                    double _add55r = 0, _add55i = 0;
                    _add55r = j + _c54r; _add55i = 0 + _c54i;
                    k = _add55r;
                    double _abs56r = 0, _abs56i = 0;
                    _abs56r = c_abs(x1r, x1i); _abs56i = 0;
                    double _c57r = 0, _c57i = 0;
                    _c57r = 1.0; _c57i = 0;
                    double _add58r = 0, _add58i = 0;
                    _add58r = _abs56r + _c57r; _add58i = _abs56i + _c57i;
                    double _log59r = 0, _log59i = 0;
                    c_log(_add58r, _add58i, &_log59r, &_log59i);
                    double _pow60r = 0, _pow60i = 0;
                    c_powr(_log59r, _log59i, k, &_pow60r, &_pow60i);
                    double _abs61r = 0, _abs61i = 0;
                    _abs61r = c_abs(x2r, x2i); _abs61i = 0;
                    double _c62r = 0, _c62i = 0;
                    _c62r = 1.0; _c62i = 0;
                    double _add63r = 0, _add63i = 0;
                    _add63r = _abs61r + _c62r; _add63i = _abs61i + _c62i;
                    double _log64r = 0, _log64i = 0;
                    c_log(_add63r, _add63i, &_log64r, &_log64i);
                    double _c65r = 0, _c65i = 0;
                    _c65r = 35.0; _c65i = 0;
                    double _sub66r = 0, _sub66i = 0;
                    _sub66r = _c65r - k; _sub66i = _c65i - 0;
                    double _pow67r = 0, _pow67i = 0;
                    c_powc(_log64r, _log64i, _sub66r, _sub66i, &_pow67r, &_pow67i);
                    double _add68r = 0, _add68i = 0;
                    _add68r = _pow60r + _pow67r; _add68i = _pow60i + _pow67i;
                    double _mul69r = 0, _mul69i = 0;
                    c_mul(x1r, x1i, x2r, x2i, &_mul69r, &_mul69i);
                    double _re70r = 0, _re70i = 0;
                    _re70r = _mul69r; _re70i = 0;
                    double _add71r = 0, _add71i = 0;
                    _add71r = x1r + x2r; _add71i = x1i + x2i;
                    double _im72r = 0, _im72i = 0;
                    _im72r = _add71i; _im72i = 0;
                    double _mul73r = 0, _mul73i = 0;
                    c_mul(_re70r, _re70i, _im72r, _im72i, &_mul73r, &_mul73i);
                    double _add74r = 0, _add74i = 0;
                    _add74r = _add68r + _mul73r; _add74i = _add68i + _mul73i;
                    { int _idx = j; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add74r; cIm[_idx] = _add74i; } }
                }
            }
        }
    }
    double _c75r = 0, _c75i = 0;
    _c75r = 2.0; _c75i = 0;
    double _c76r = 0, _c76i = 0;
    _c76r = 0.0; _c76i = 3.0;
    double _add77r = 0, _add77i = 0;
    _add77r = _c75r + _c76r; _add77i = _c75i + _c76i;
    { int _idx = 4; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add77r; cIm[_idx] = _add77i; } }
    double _c78r = 0, _c78i = 0;
    _c78r = 0.0; _c78i = 4.0;
    double _neg79r = 0, _neg79i = 0;
    _neg79r = -(_c78r); _neg79i = -(_c78i);
    { int _idx = 9; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _neg79r; cIm[_idx] = _neg79i; } }
    double _c80r = 0, _c80i = 0;
    _c80r = 5.0; _c80i = 0;
    double _c81r = 0, _c81i = 0;
    _c81r = 0.0; _c81i = 6.0;
    double _sub82r = 0, _sub82i = 0;
    _sub82r = _c80r - _c81r; _sub82i = _c80i - _c81i;
    { int _idx = 14; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub82r; cIm[_idx] = _sub82i; } }
    double _c83r = 0, _c83i = 0;
    _c83r = 7.0; _c83i = 0;
    double _neg84r = 0, _neg84i = 0;
    _neg84r = -(_c83r); _neg84i = -(_c83i);
    double _c85r = 0, _c85i = 0;
    _c85r = 0.0; _c85i = 8.0;
    double _add86r = 0, _add86i = 0;
    _add86r = _neg84r + _c85r; _add86i = _neg84i + _c85i;
    { int _idx = 19; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add86r; cIm[_idx] = _add86i; } }
    double _c87r = 0, _c87i = 0;
    _c87r = 9.0; _c87i = 0;
    double _c88r = 0, _c88i = 0;
    _c88r = 0.0; _c88i = 10.0;
    double _sub89r = 0, _sub89i = 0;
    _sub89r = _c87r - _c88r; _sub89i = _c87i - _c88i;
    { int _idx = 24; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub89r; cIm[_idx] = _sub89i; } }
    double _c90r = 0, _c90i = 0;
    _c90r = 11.0; _c90i = 0;
    double _c91r = 0, _c91i = 0;
    _c91r = 0.0; _c91i = 12.0;
    double _add92r = 0, _add92i = 0;
    _add92r = _c90r + _c91r; _add92i = _c90i + _c91i;
    { int _idx = 29; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add92r; cIm[_idx] = _add92i; } }
    double _c93r = 0, _c93i = 0;
    _c93r = 13.0; _c93i = 0;
    double _neg94r = 0, _neg94i = 0;
    _neg94r = -(_c93r); _neg94i = -(_c93i);
    double _c95r = 0, _c95i = 0;
    _c95r = 0.0; _c95i = 14.0;
    double _add96r = 0, _add96i = 0;
    _add96r = _neg94r + _c95r; _add96i = _neg94i + _c95i;
    { int _idx = 34; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add96r; cIm[_idx] = _add96i; } }
    double _c97r = 0, _c97i = 0;
    _c97r = 0.0; _c97i = 100.0;
    double _c98r = 0, _c98i = 0;
    _c98r = 3.0; _c98i = 0;
    double _pow99r = 0, _pow99i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow99r, &_pow99i);
    c_mul(_pow99r, _pow99i, x2r, x2i, &_pow99r, &_pow99i);
    double _mul100r = 0, _mul100i = 0;
    c_mul(_c97r, _c97i, _pow99r, _pow99i, &_mul100r, &_mul100i);
    double _c101r = 0, _c101i = 0;
    _c101r = 0.0; _c101i = 100.0;
    double _c102r = 0, _c102i = 0;
    _c102r = 2.0; _c102i = 0;
    double _pow103r = 0, _pow103i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow103r, &_pow103i);
    double _mul104r = 0, _mul104i = 0;
    c_mul(_c101r, _c101i, _pow103r, _pow103i, &_mul104r, &_mul104i);
    double _add105r = 0, _add105i = 0;
    _add105r = _mul100r + _mul104r; _add105i = _mul100i + _mul104i;
    double _c106r = 0, _c106i = 0;
    _c106r = 100.0; _c106i = 0;
    double _mul107r = 0, _mul107i = 0;
    c_mul(_c106r, _c106i, x2r, x2i, &_mul107r, &_mul107i);
    double _sub108r = 0, _sub108i = 0;
    _sub108r = _add105r - _mul107r; _sub108i = _add105i - _mul107i;
    double _c109r = 0, _c109i = 0;
    _c109r = 100.0; _c109i = 0;
    double _sub110r = 0, _sub110i = 0;
    _sub110r = _sub108r - _c109r; _sub110i = _sub108i - _c109i;
    { int _idx = 7; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub110r; cIm[_idx] = _sub110i; } }
    double _c111r = 0, _c111i = 0;
    _c111r = 0.0; _c111i = 150.0;
    double _c112r = 0, _c112i = 0;
    _c112r = 3.0; _c112i = 0;
    double _pow113r = 0, _pow113i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow113r, &_pow113i);
    c_mul(_pow113r, _pow113i, x1r, x1i, &_pow113r, &_pow113i);
    double _mul114r = 0, _mul114i = 0;
    c_mul(_c111r, _c111i, _pow113r, _pow113i, &_mul114r, &_mul114i);
    double _c115r = 0, _c115i = 0;
    _c115r = 0.0; _c115i = 150.0;
    double _c116r = 0, _c116i = 0;
    _c116r = 2.0; _c116i = 0;
    double _pow117r = 0, _pow117i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow117r, &_pow117i);
    double _mul118r = 0, _mul118i = 0;
    c_mul(_c115r, _c115i, _pow117r, _pow117i, &_mul118r, &_mul118i);
    double _add119r = 0, _add119i = 0;
    _add119r = _mul114r + _mul118r; _add119i = _mul114i + _mul118i;
    double _c120r = 0, _c120i = 0;
    _c120r = 150.0; _c120i = 0;
    double _mul121r = 0, _mul121i = 0;
    c_mul(_c120r, _c120i, x2r, x2i, &_mul121r, &_mul121i);
    double _add122r = 0, _add122i = 0;
    _add122r = _add119r + _mul121r; _add122i = _add119i + _mul121i;
    double _c123r = 0, _c123i = 0;
    _c123r = 150.0; _c123i = 0;
    double _sub124r = 0, _sub124i = 0;
    _sub124r = _add122r - _c123r; _sub124i = _add122i - _c123i;
    { int _idx = 11; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub124r; cIm[_idx] = _sub124i; } }
    double _c125r = 0, _c125i = 0;
    _c125r = 0.0; _c125i = 200.0;
    double _c126r = 0, _c126i = 0;
    _c126r = 3.0; _c126i = 0;
    double _pow127r = 0, _pow127i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow127r, &_pow127i);
    c_mul(_pow127r, _pow127i, x2r, x2i, &_pow127r, &_pow127i);
    double _mul128r = 0, _mul128i = 0;
    c_mul(_c125r, _c125i, _pow127r, _pow127i, &_mul128r, &_mul128i);
    double _c129r = 0, _c129i = 0;
    _c129r = 0.0; _c129i = 200.0;
    double _c130r = 0, _c130i = 0;
    _c130r = 2.0; _c130i = 0;
    double _pow131r = 0, _pow131i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow131r, &_pow131i);
    double _mul132r = 0, _mul132i = 0;
    c_mul(_c129r, _c129i, _pow131r, _pow131i, &_mul132r, &_mul132i);
    double _sub133r = 0, _sub133i = 0;
    _sub133r = _mul128r - _mul132r; _sub133i = _mul128i - _mul132i;
    double _c134r = 0, _c134i = 0;
    _c134r = 200.0; _c134i = 0;
    double _mul135r = 0, _mul135i = 0;
    c_mul(_c134r, _c134i, x2r, x2i, &_mul135r, &_mul135i);
    double _add136r = 0, _add136i = 0;
    _add136r = _sub133r + _mul135r; _add136i = _sub133i + _mul135i;
    double _c137r = 0, _c137i = 0;
    _c137r = 200.0; _c137i = 0;
    double _sub138r = 0, _sub138i = 0;
    _sub138r = _add136r - _c137r; _sub138i = _add136i - _c137i;
    { int _idx = 17; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub138r; cIm[_idx] = _sub138i; } }
    double _c139r = 0, _c139i = 0;
    _c139r = 250.0; _c139i = 0;
    double _sin140r = 0, _sin140i = 0;
    c_sin(x1r, x1i, &_sin140r, &_sin140i);
    double _mul141r = 0, _mul141i = 0;
    c_mul(_c139r, _c139i, _sin140r, _sin140i, &_mul141r, &_mul141i);
    double _c142r = 0, _c142i = 0;
    _c142r = 0.0; _c142i = 300.0;
    double _cos143r = 0, _cos143i = 0;
    c_cos(x2r, x2i, &_cos143r, &_cos143i);
    double _mul144r = 0, _mul144i = 0;
    c_mul(_c142r, _c142i, _cos143r, _cos143i, &_mul144r, &_mul144i);
    double _add145r = 0, _add145i = 0;
    _add145r = _mul141r + _mul144r; _add145i = _mul141i + _mul144i;
    double _c146r = 0, _c146i = 0;
    _c146r = 50.0; _c146i = 0;
    double _abs147r = 0, _abs147i = 0;
    _abs147r = c_abs(x1r, x1i); _abs147i = 0;
    double _c148r = 0, _c148i = 0;
    _c148r = 1.0; _c148i = 0;
    double _add149r = 0, _add149i = 0;
    _add149r = _abs147r + _c148r; _add149i = _abs147i + _c148i;
    double _log150r = 0, _log150i = 0;
    c_log(_add149r, _add149i, &_log150r, &_log150i);
    double _mul151r = 0, _mul151i = 0;
    c_mul(_c146r, _c146i, _log150r, _log150i, &_mul151r, &_mul151i);
    double _add152r = 0, _add152i = 0;
    _add152r = _add145r + _mul151r; _add152i = _add145i + _mul151i;
    { int _idx = 21; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add152r; cIm[_idx] = _add152i; } }
    double _c153r = 0, _c153i = 0;
    _c153r = 350.0; _c153i = 0;
    double _prod154r = 0, _prod154i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_prod154r, &_prod154i);
    double _mul155r = 0, _mul155i = 0;
    c_mul(_c153r, _c153i, _prod154r, _prod154i, &_mul155r, &_mul155i);
    double _c156r = 0, _c156i = 0;
    _c156r = 0.0; _c156i = 400.0;
    double _sum157r = 0, _sum157i = 0;
    _sum157r = x1r + x2r; _sum157i = x1i + x2i;
    double _mul158r = 0, _mul158i = 0;
    c_mul(_c156r, _c156i, _sum157r, _sum157i, &_mul158r, &_mul158i);
    double _add159r = 0, _add159i = 0;
    _add159r = _mul155r + _mul158r; _add159i = _mul155i + _mul158i;
    { int _idx = 27; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add159r; cIm[_idx] = _add159i; } }
    double _c160r = 0, _c160i = 0;
    _c160r = 0.0; _c160i = 450.0;
    double _mul161r = 0, _mul161i = 0;
    c_mul(_c160r, _c160i, x1r, x1i, &_mul161r, &_mul161i);
    double _mul162r = 0, _mul162i = 0;
    c_mul(_mul161r, _mul161i, x2r, x2i, &_mul162r, &_mul162i);
    double _c163r = 0, _c163i = 0;
    _c163r = 500.0; _c163i = 0;
    double _sub164r = 0, _sub164i = 0;
    _sub164r = x1r - x2r; _sub164i = x1i - x2i;
    double _conj165r = 0, _conj165i = 0;
    _conj165r = _sub164r; _conj165i = -(_sub164i);
    double _mul166r = 0, _mul166i = 0;
    c_mul(_c163r, _c163i, _conj165r, _conj165i, &_mul166r, &_mul166i);
    double _add167r = 0, _add167i = 0;
    _add167r = _mul162r + _mul166r; _add167i = _mul162i + _mul166i;
    { int _idx = 32; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add167r; cIm[_idx] = _add167i; } }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_225: auto-stubbed (unhandled constructs in source) */
static void poly_225_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_226: auto-stubbed (unhandled constructs in source) */
static void poly_226_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_227_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 0; j < 35; j++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 1.0; _c1i = 0;
        double _add2r = 0, _add2i = 0;
        _add2r = j + _c1r; _add2i = 0 + _c1i;
        double _c3r = 0, _c3i = 0;
        _c3r = 5.0; _c3i = 0;
        double _fdiv4r = 0, _fdiv4i = 0;
        c_div(j, 0, _c3r, _c3i, &_fdiv4r, &_fdiv4i);
        _fdiv4r = floor(_fdiv4r); _fdiv4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = _add2r + _fdiv4r; _add5i = _add2i + _fdiv4i;
        double k_r = _add5r, k_i = _add5i;
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
        _add11r = j + _c10r; _add11i = 0 + _c10i;
        double _sin12r = 0, _sin12i = 0;
        c_sin(_add11r, _add11i, &_sin12r, &_sin12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_log9r, _log9i, _sin12r, _sin12i, &_mul13r, &_mul13i);
        double _abs14r = 0, _abs14i = 0;
        _abs14r = c_abs(x2r, x2i); _abs14i = 0;
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
        double _cos20r = 0, _cos20i = 0;
        c_cos(_add19r, _add19i, &_cos20r, &_cos20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_log17r, _log17i, _cos20r, _cos20i, &_mul21r, &_mul21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _mul13r + _mul21r; _add22i = _mul13i + _mul21i;
        double magnitude_r = _add22r, magnitude_i = _add22i;
        double _ang23r = 0, _ang23i = 0;
        _ang23r = c_arg(x1r, x1i); _ang23i = 0;
        double _c24r = 0, _c24i = 0;
        _c24r = 1.0; _c24i = 0;
        double _add25r = 0, _add25i = 0;
        _add25r = j + _c24r; _add25i = 0 + _c24i;
        double _c26r = 0, _c26i = 0;
        _c26r = 0.5; _c26i = 0;
        double _pow27r = 0, _pow27i = 0;
        c_powr(_add25r, _add25i, 0.5, &_pow27r, &_pow27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_ang23r, _ang23i, _pow27r, _pow27i, &_mul28r, &_mul28i);
        double _ang29r = 0, _ang29i = 0;
        _ang29r = c_arg(x2r, x2i); _ang29i = 0;
        double _c30r = 0, _c30i = 0;
        _c30r = 2.0; _c30i = 0;
        double _add31r = 0, _add31i = 0;
        _add31r = j + _c30r; _add31i = 0 + _c30i;
        double _log32r = 0, _log32i = 0;
        c_log(_add31r, _add31i, &_log32r, &_log32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_ang29r, _ang29i, _log32r, _log32i, &_mul33r, &_mul33i);
        double _sub34r = 0, _sub34i = 0;
        _sub34r = _mul28r - _mul33r; _sub34i = _mul28i - _mul33i;
        double angle_r = _sub34r, angle_i = _sub34i;
        double _cos35r = 0, _cos35i = 0;
        c_cos(angle_r, angle_i, &_cos35r, &_cos35i);
        double _c36r = 0, _c36i = 0;
        _c36r = 0.0; _c36i = 1.0;
        double _sin37r = 0, _sin37i = 0;
        c_sin(angle_r, angle_i, &_sin37r, &_sin37i);
        double _mul38r = 0, _mul38i = 0;
        c_mul(_c36r, _c36i, _sin37r, _sin37i, &_mul38r, &_mul38i);
        double _add39r = 0, _add39i = 0;
        _add39r = _cos35r + _mul38r; _add39i = _cos35i + _mul38i;
        double _mul40r = 0, _mul40i = 0;
        c_mul(magnitude_r, magnitude_i, _add39r, _add39i, &_mul40r, &_mul40i);
        double _conj41r = 0, _conj41i = 0;
        _conj41r = x1r; _conj41i = -(x1i);
        double _pow42r = 0, _pow42i = 0;
        c_powc(_conj41r, _conj41i, k_r, k_i, &_pow42r, &_pow42i);
        double _add43r = 0, _add43i = 0;
        _add43r = _mul40r + _pow42r; _add43i = _mul40i + _pow42i;
        double _conj44r = 0, _conj44i = 0;
        _conj44r = x2r; _conj44i = -(x2i);
        double _c45r = 0, _c45i = 0;
        _c45r = 35.0; _c45i = 0;
        double _sub46r = 0, _sub46i = 0;
        _sub46r = _c45r - j; _sub46i = _c45i - 0;
        double _pow47r = 0, _pow47i = 0;
        c_powc(_conj44r, _conj44i, _sub46r, _sub46i, &_pow47r, &_pow47i);
        double _sub48r = 0, _sub48i = 0;
        _sub48r = _add43r - _pow47r; _sub48i = _add43i - _pow47i;
        { int _idx = j; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub48r; cIm[_idx] = _sub48i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_228: auto-stubbed (unhandled constructs in source) */
static void poly_228_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_229: auto-stubbed (unhandled constructs in source) */
static void poly_229_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_230_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 3.0; _c1i = 0;
    { int _idx = 2; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _c1r; cIm[_idx] = _c1i; } }
    double _c2r = 0, _c2i = 0;
    _c2r = 2.0; _c2i = 0;
    double _neg3r = 0, _neg3i = 0;
    _neg3r = -(_c2r); _neg3i = -(_c2i);
    { int _idx = 7; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _neg3r; cIm[_idx] = _neg3i; } }
    double _c4r = 0, _c4i = 0;
    _c4r = 5.0; _c4i = 0;
    { int _idx = 11; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _c4r; cIm[_idx] = _c4i; } }
    double _c5r = 0, _c5i = 0;
    _c5r = 4.0; _c5i = 0;
    double _neg6r = 0, _neg6i = 0;
    _neg6r = -(_c5r); _neg6i = -(_c5i);
    { int _idx = 16; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _neg6r; cIm[_idx] = _neg6i; } }
    double _c7r = 0, _c7i = 0;
    _c7r = 6.0; _c7i = 0;
    { int _idx = 22; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _c7r; cIm[_idx] = _c7i; } }
    double _c8r = 0, _c8i = 0;
    _c8r = 3.0; _c8i = 0;
    double _neg9r = 0, _neg9i = 0;
    _neg9r = -(_c8r); _neg9i = -(_c8i);
    { int _idx = 27; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _neg9r; cIm[_idx] = _neg9i; } }
    double _c10r = 0, _c10i = 0;
    _c10r = 2.0; _c10i = 0;
    { int _idx = 31; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _c10r; cIm[_idx] = _c10i; } }
    double _c11r = 0, _c11i = 0;
    _c11r = 1.0; _c11i = 0;
    double _neg12r = 0, _neg12i = 0;
    _neg12r = -(_c11r); _neg12i = -(_c11i);
    { int _idx = 34; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _neg12r; cIm[_idx] = _neg12i; } }
    for (int j = 0; j < 35; j++) {
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = j + _c13r; _add14i = 0 + _c13i;
        double _c15r = 0, _c15i = 0;
        _c15r = 4.0; _c15i = 0;
        double _mod16r = 0, _mod16i = 0;
        _mod16r = fmod(_add14r, _c15r); _mod16i = 0;
        double _c17r = 0, _c17i = 0;
        _c17r = 0.0; _c17i = 0;
        if (_mod16r == _c17r) {
            double _re18r = 0, _re18i = 0;
            _re18r = x1r; _re18i = 0;
            double _c19r = 0, _c19i = 0;
            _c19r = 2.0; _c19i = 0;
            double _pow20r = 0, _pow20i = 0;
            c_mul(_re18r, _re18i, _re18r, _re18i, &_pow20r, &_pow20i);
            double _im21r = 0, _im21i = 0;
            _im21r = x2i; _im21i = 0;
            double _add22r = 0, _add22i = 0;
            _add22r = _pow20r + _im21r; _add22i = _pow20i + _im21i;
            double _c23r = 0, _c23i = 0;
            _c23r = 1.0; _c23i = 0;
            double _add24r = 0, _add24i = 0;
            _add24r = j + _c23r; _add24i = 0 + _c23i;
            double _mul25r = 0, _mul25i = 0;
            c_mul(_add22r, _add22i, _add24r, _add24i, &_mul25r, &_mul25i);
            double _im26r = 0, _im26i = 0;
            _im26r = x1i; _im26i = 0;
            double _re27r = 0, _re27i = 0;
            _re27r = x2r; _re27i = 0;
            double _mul28r = 0, _mul28i = 0;
            c_mul(_im26r, _im26i, _re27r, _re27i, &_mul28r, &_mul28i);
            double _c29r = 0, _c29i = 0;
            _c29r = 0.0; _c29i = 1.0;
            double _mul30r = 0, _mul30i = 0;
            c_mul(_mul28r, _mul28i, _c29r, _c29i, &_mul30r, &_mul30i);
            double _add31r = 0, _add31i = 0;
            _add31r = _mul25r + _mul30r; _add31i = _mul25i + _mul30i;
            { int _idx = j; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add31r; cIm[_idx] = _add31i; } }
        } else {
            double _c32r = 0, _c32i = 0;
            _c32r = 1.0; _c32i = 0;
            double _add33r = 0, _add33i = 0;
            _add33r = j + _c32r; _add33i = 0 + _c32i;
            double _c34r = 0, _c34i = 0;
            _c34r = 5.0; _c34i = 0;
            double _mod35r = 0, _mod35i = 0;
            _mod35r = fmod(_add33r, _c34r); _mod35i = 0;
            double _c36r = 0, _c36i = 0;
            _c36r = 1.0; _c36i = 0;
            if (_mod35r == _c36r) {
                double _c37r = 0, _c37i = 0;
                _c37r = 1.0; _c37i = 0;
                double _add38r = 0, _add38i = 0;
                _add38r = j + _c37r; _add38i = 0 + _c37i;
                double _mul39r = 0, _mul39i = 0;
                c_mul(x1r, x1i, _add38r, _add38i, &_mul39r, &_mul39i);
                double _sin40r = 0, _sin40i = 0;
                c_sin(_mul39r, _mul39i, &_sin40r, &_sin40i);
                double _c41r = 0, _c41i = 0;
                _c41r = 1.0; _c41i = 0;
                double _add42r = 0, _add42i = 0;
                _add42r = j + _c41r; _add42i = 0 + _c41i;
                double _add43r = 0, _add43i = 0;
                _add43r = x2r + _add42r; _add43i = x2i + _add42i;
                double _cos44r = 0, _cos44i = 0;
                c_cos(_add43r, _add43i, &_cos44r, &_cos44i);
                double _c45r = 0, _c45i = 0;
                _c45r = 0.0; _c45i = 1.0;
                double _mul46r = 0, _mul46i = 0;
                c_mul(_cos44r, _cos44i, _c45r, _c45i, &_mul46r, &_mul46i);
                double _add47r = 0, _add47i = 0;
                _add47r = _sin40r + _mul46r; _add47i = _sin40i + _mul46i;
                { int _idx = j; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add47r; cIm[_idx] = _add47i; } }
            } else {
                double _c48r = 0, _c48i = 0;
                _c48r = 1.0; _c48i = 0;
                double _add49r = 0, _add49i = 0;
                _add49r = j + _c48r; _add49i = 0 + _c48i;
                double _c50r = 0, _c50i = 0;
                _c50r = 3.0; _c50i = 0;
                double _mod51r = 0, _mod51i = 0;
                _mod51r = fmod(_add49r, _c50r); _mod51i = 0;
                double _c52r = 0, _c52i = 0;
                _c52r = 2.0; _c52i = 0;
                if (_mod51r == _c52r) {
                    double _abs53r = 0, _abs53i = 0;
                    _abs53r = c_abs(x1r, x1i); _abs53i = 0;
                    double _c54r = 0, _c54i = 0;
                    _c54r = 1.0; _c54i = 0;
                    double _add55r = 0, _add55i = 0;
                    _add55r = j + _c54r; _add55i = 0 + _c54i;
                    double _mul56r = 0, _mul56i = 0;
                    c_mul(_abs53r, _abs53i, _add55r, _add55i, &_mul56r, &_mul56i);
                    double _c57r = 0, _c57i = 0;
                    _c57r = 1.0; _c57i = 0;
                    double _add58r = 0, _add58i = 0;
                    _add58r = _mul56r + _c57r; _add58i = _mul56i + _c57i;
                    double _log59r = 0, _log59i = 0;
                    c_log(_add58r, _add58i, &_log59r, &_log59i);
                    double _ang60r = 0, _ang60i = 0;
                    _ang60r = c_arg(x2r, x2i); _ang60i = 0;
                    double _c61r = 0, _c61i = 0;
                    _c61r = 1.0; _c61i = 0;
                    double _add62r = 0, _add62i = 0;
                    _add62r = j + _c61r; _add62i = 0 + _c61i;
                    double _pow63r = 0, _pow63i = 0;
                    c_powc(_ang60r, _ang60i, _add62r, _add62i, &_pow63r, &_pow63i);
                    double _c64r = 0, _c64i = 0;
                    _c64r = 0.0; _c64i = 1.0;
                    double _mul65r = 0, _mul65i = 0;
                    c_mul(_pow63r, _pow63i, _c64r, _c64i, &_mul65r, &_mul65i);
                    double _add66r = 0, _add66i = 0;
                    _add66r = _log59r + _mul65r; _add66i = _log59i + _mul65i;
                    { int _idx = j; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add66r; cIm[_idx] = _add66i; } }
                } else {
                    double _add67r = 0, _add67i = 0;
                    _add67r = x1r + x2r; _add67i = x1i + x2i;
                    double _re68r = 0, _re68i = 0;
                    _re68r = _add67r; _re68i = 0;
                    double _c69r = 0, _c69i = 0;
                    _c69r = 1.0; _c69i = 0;
                    double _add70r = 0, _add70i = 0;
                    _add70r = j + _c69r; _add70i = 0 + _c69i;
                    double _mul71r = 0, _mul71i = 0;
                    c_mul(_re68r, _re68i, _add70r, _add70i, &_mul71r, &_mul71i);
                    double _sub72r = 0, _sub72i = 0;
                    _sub72r = x1r - x2r; _sub72i = x1i - x2i;
                    double _im73r = 0, _im73i = 0;
                    _im73r = _sub72i; _im73i = 0;
                    double _c74r = 0, _c74i = 0;
                    _c74r = 0.0; _c74i = 1.0;
                    double _mul75r = 0, _mul75i = 0;
                    c_mul(_im73r, _im73i, _c74r, _c74i, &_mul75r, &_mul75i);
                    double _add76r = 0, _add76i = 0;
                    _add76r = _mul71r + _mul75r; _add76i = _mul71i + _mul75i;
                    { int _idx = j; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add76r; cIm[_idx] = _add76i; } }
                }
            }
        }
    }
    for (int k = 1; k < 6; k++) {
        for (int r = 1; r < 8; r++) {
            double _mul77r = 0, _mul77i = 0;
            c_mul(k, 0, r, 0, &_mul77r, &_mul77i);
            double _c78r = 0, _c78i = 0;
            _c78r = 35.0; _c78i = 0;
            double _mod79r = 0, _mod79i = 0;
            _mod79r = fmod(_mul77r, _c78r); _mod79i = 0;
            double idx_r = _mod79r, idx_i = _mod79i;
            double _re80r = 0, _re80i = 0;
            _re80r = x1r; _re80i = 0;
            double _pow81r = 0, _pow81i = 0;
            c_powr(_re80r, _re80i, k, &_pow81r, &_pow81i);
            double _im82r = 0, _im82i = 0;
            _im82r = x2i; _im82i = 0;
            double _pow83r = 0, _pow83i = 0;
            c_powr(_im82r, _im82i, r, &_pow83r, &_pow83i);
            double _mul84r = 0, _mul84i = 0;
            c_mul(_pow81r, _pow81i, _pow83r, _pow83i, &_mul84r, &_mul84i);
            double _re85r = 0, _re85i = 0;
            _re85r = x2r; _re85i = 0;
            double _pow86r = 0, _pow86i = 0;
            c_powr(_re85r, _re85i, k, &_pow86r, &_pow86i);
            double _im87r = 0, _im87i = 0;
            _im87r = x1i; _im87i = 0;
            double _pow88r = 0, _pow88i = 0;
            c_powr(_im87r, _im87i, r, &_pow88r, &_pow88i);
            double _mul89r = 0, _mul89i = 0;
            c_mul(_pow86r, _pow86i, _pow88r, _pow88i, &_mul89r, &_mul89i);
            double _c90r = 0, _c90i = 0;
            _c90r = 0.0; _c90i = 1.0;
            double _mul91r = 0, _mul91i = 0;
            c_mul(_mul89r, _mul89i, _c90r, _c90i, &_mul91r, &_mul91i);
            double _add92r = 0, _add92i = 0;
            _add92r = _mul84r + _mul91r; _add92i = _mul84i + _mul91i;
            cRe[(int)(idx_r)] += _add92r; cIm[(int)(idx_r)] += _add92i;
        }
    }
    for (int m = 10; m < 31; m += 5) {
        double _cf93r = 0, _cf93i = 0;
        { int _idx = m; if (_idx >= 0 && _idx < 35) { _cf93r = cRe[_idx]; _cf93i = cIm[_idx]; } }
        double _conj94r = 0, _conj94i = 0;
        _conj94r = x1r; _conj94i = -(x1i);
        double _mul95r = 0, _mul95i = 0;
        c_mul(_cf93r, _cf93i, _conj94r, _conj94i, &_mul95r, &_mul95i);
        double _abs96r = 0, _abs96i = 0;
        _abs96r = c_abs(x2r, x2i); _abs96i = 0;
        double _prod97r = 0, _prod97i = 0;
        c_mul(_abs96r, _abs96i, m, 0, &_prod97r, &_prod97i);
        double _c98r = 0, _c98i = 0;
        _c98r = 0.0; _c98i = 1.0;
        double _mul99r = 0, _mul99i = 0;
        c_mul(_prod97r, _prod97i, _c98r, _c98i, &_mul99r, &_mul99i);
        double _add100r = 0, _add100i = 0;
        _add100r = _mul95r + _mul99r; _add100i = _mul95i + _mul99i;
        { int _idx = m; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add100r; cIm[_idx] = _add100i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_231_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 50;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 50; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 0; k < 50; k++) {
        double _abs1r = 0, _abs1i = 0;
        _abs1r = c_abs(x1r, x1i); _abs1i = 0;
        double _c2r = 0, _c2i = 0;
        _c2r = 5.0; _c2i = 0;
        double _mod3r = 0, _mod3i = 0;
        _mod3r = fmod(k, _c2r); _mod3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 1.0; _c4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = _mod3r + _c4r; _add5i = _mod3i + _c4i;
        double _pow6r = 0, _pow6i = 0;
        c_powc(_abs1r, _abs1i, _add5r, _add5i, &_pow6r, &_pow6i);
        double _abs7r = 0, _abs7i = 0;
        _abs7r = c_abs(x2r, x2i); _abs7i = 0;
        double _c8r = 0, _c8i = 0;
        _c8r = 7.0; _c8i = 0;
        double _mod9r = 0, _mod9i = 0;
        _mod9r = fmod(k, _c8r); _mod9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _mod9r + _c10r; _add11i = _mod9i + _c10i;
        double _pow12r = 0, _pow12i = 0;
        c_powc(_abs7r, _abs7i, _add11r, _add11i, &_pow12r, &_pow12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _pow6r + _pow12r; _add13i = _pow6i + _pow12i;
        double _abs14r = 0, _abs14i = 0;
        _abs14r = c_abs(x1r, x1i); _abs14i = 0;
        double _c15r = 0, _c15i = 0;
        _c15r = 1.0; _c15i = 0;
        double _add16r = 0, _add16i = 0;
        _add16r = _abs14r + _c15r; _add16i = _abs14i + _c15i;
        double _log17r = 0, _log17i = 0;
        c_log(_add16r, _add16i, &_log17r, &_log17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 1.0; _c18i = 0;
        double _add19r = 0, _add19i = 0;
        _add19r = k + _c18r; _add19i = 0 + _c18i;
        double _sin20r = 0, _sin20i = 0;
        c_sin(_add19r, _add19i, &_sin20r, &_sin20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_log17r, _log17i, _sin20r, _sin20i, &_mul21r, &_mul21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _add13r + _mul21r; _add22i = _add13i + _mul21i;
        double magnitude_r = _add22r, magnitude_i = _add22i;
        double _ang23r = 0, _ang23i = 0;
        _ang23r = c_arg(x1r, x1i); _ang23i = 0;
        double _c24r = 0, _c24i = 0;
        _c24r = 1.0; _c24i = 0;
        double _add25r = 0, _add25i = 0;
        _add25r = k + _c24r; _add25i = 0 + _c24i;
        double _cos26r = 0, _cos26i = 0;
        c_cos(_add25r, _add25i, &_cos26r, &_cos26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_ang23r, _ang23i, _cos26r, _cos26i, &_mul27r, &_mul27i);
        double _ang28r = 0, _ang28i = 0;
        _ang28r = c_arg(x2r, x2i); _ang28i = 0;
        double _c29r = 0, _c29i = 0;
        _c29r = 1.0; _c29i = 0;
        double _add30r = 0, _add30i = 0;
        _add30r = k + _c29r; _add30i = 0 + _c29i;
        double _sin31r = 0, _sin31i = 0;
        c_sin(_add30r, _add30i, &_sin31r, &_sin31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_ang28r, _ang28i, _sin31r, _sin31i, &_mul32r, &_mul32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _mul27r + _mul32r; _add33i = _mul27i + _mul32i;
        double angle_r = _add33r, angle_i = _add33i;
        double _cos34r = 0, _cos34i = 0;
        c_cos(angle_r, angle_i, &_cos34r, &_cos34i);
        double _sin35r = 0, _sin35i = 0;
        c_sin(angle_r, angle_i, &_sin35r, &_sin35i);
        double _c36r = 0, _c36i = 0;
        _c36r = 0.0; _c36i = 1.0;
        double _mul37r = 0, _mul37i = 0;
        c_mul(_sin35r, _sin35i, _c36r, _c36i, &_mul37r, &_mul37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _cos34r + _mul37r; _add38i = _cos34i + _mul37i;
        double _mul39r = 0, _mul39i = 0;
        c_mul(magnitude_r, magnitude_i, _add38r, _add38i, &_mul39r, &_mul39i);
        { int _idx = k; if (_idx >= 0 && _idx < 50) { cRe[_idx] = _mul39r; cIm[_idx] = _mul39i; } }
    }
    for (int j = 2; j < 51; j += 3) {
        double _conj40r = 0, _conj40i = 0;
        _conj40r = x1r; _conj40i = -(x1i);
        double _c41r = 0, _c41i = 0;
        _c41r = 4.0; _c41i = 0;
        double _mod42r = 0, _mod42i = 0;
        _mod42r = fmod(j, _c41r); _mod42i = 0;
        double _pow43r = 0, _pow43i = 0;
        c_powc(x2r, x2i, _mod42r, _mod42i, &_pow43r, &_pow43i);
        double _mul44r = 0, _mul44i = 0;
        c_mul(_conj40r, _conj40i, _pow43r, _pow43i, &_mul44r, &_mul44i);
        cRe[j] += _mul44r; cIm[j] += _mul44i;
    }
    for (int r = 5; r < 51; r += 5) {
        double _re45r = 0, _re45i = 0;
        _re45r = x2r; _re45i = 0;
        double _cos46r = 0, _cos46i = 0;
        c_cos(r, 0, &_cos46r, &_cos46i);
        double _mul47r = 0, _mul47i = 0;
        c_mul(_re45r, _re45i, _cos46r, _cos46i, &_mul47r, &_mul47i);
        double _im48r = 0, _im48i = 0;
        _im48r = x1i; _im48i = 0;
        double _sin49r = 0, _sin49i = 0;
        c_sin(r, 0, &_sin49r, &_sin49i);
        double _mul50r = 0, _mul50i = 0;
        c_mul(_im48r, _im48i, _sin49r, _sin49i, &_mul50r, &_mul50i);
        double _c51r = 0, _c51i = 0;
        _c51r = 0.0; _c51i = 1.0;
        double _mul52r = 0, _mul52i = 0;
        c_mul(_mul50r, _mul50i, _c51r, _c51i, &_mul52r, &_mul52i);
        double _add53r = 0, _add53i = 0;
        _add53r = _mul47r + _mul52r; _add53i = _mul47i + _mul52i;
        cRe[r] += _add53r; cIm[r] += _add53i;
    }
    double _abs54r = 0, _abs54i = 0;
    _abs54r = c_abs(x1r, x1i); _abs54i = 0;
    double _abs55r = 0, _abs55i = 0;
    _abs55r = c_abs(x2r, x2i); _abs55i = 0;
    double _sum56r = 0, _sum56i = 0;
    _sum56r = _abs54r + _abs55r; _sum56i = _abs54i + _abs55i;
    double _re57r = 0, _re57i = 0;
    _re57r = x1r; _re57i = 0;
    double _re58r = 0, _re58i = 0;
    _re58r = x2r; _re58i = 0;
    double _prod59r = 0, _prod59i = 0;
    c_mul(_re57r, _re57i, _re58r, _re58i, &_prod59r, &_prod59i);
    double _c60r = 0, _c60i = 0;
    _c60r = 0.0; _c60i = 1.0;
    double _mul61r = 0, _mul61i = 0;
    c_mul(_prod59r, _prod59i, _c60r, _c60i, &_mul61r, &_mul61i);
    double _add62r = 0, _add62i = 0;
    _add62r = _sum56r + _mul61r; _add62i = _sum56i + _mul61i;
    { int _idx = 9; if (_idx >= 0 && _idx < 50) { cRe[_idx] = _add62r; cIm[_idx] = _add62i; } }
    double _re63r = 0, _re63i = 0;
    _re63r = x1r; _re63i = 0;
    double _c64r = 0, _c64i = 0;
    _c64r = 2.0; _c64i = 0;
    double _pow65r = 0, _pow65i = 0;
    c_mul(_re63r, _re63i, _re63r, _re63i, &_pow65r, &_pow65i);
    double _im66r = 0, _im66i = 0;
    _im66r = x2i; _im66i = 0;
    double _c67r = 0, _c67i = 0;
    _c67r = 2.0; _c67i = 0;
    double _pow68r = 0, _pow68i = 0;
    c_mul(_im66r, _im66i, _im66r, _im66i, &_pow68r, &_pow68i);
    double _sub69r = 0, _sub69i = 0;
    _sub69r = _pow65r - _pow68r; _sub69i = _pow65i - _pow68i;
    double _c70r = 0, _c70i = 0;
    _c70r = 2.0; _c70i = 0;
    double _re71r = 0, _re71i = 0;
    _re71r = x1r; _re71i = 0;
    double _mul72r = 0, _mul72i = 0;
    c_mul(_c70r, _c70i, _re71r, _re71i, &_mul72r, &_mul72i);
    double _im73r = 0, _im73i = 0;
    _im73r = x2i; _im73i = 0;
    double _mul74r = 0, _mul74i = 0;
    c_mul(_mul72r, _mul72i, _im73r, _im73i, &_mul74r, &_mul74i);
    double _c75r = 0, _c75i = 0;
    _c75r = 0.0; _c75i = 1.0;
    double _mul76r = 0, _mul76i = 0;
    c_mul(_mul74r, _mul74i, _c75r, _c75i, &_mul76r, &_mul76i);
    double _add77r = 0, _add77i = 0;
    _add77r = _sub69r + _mul76r; _add77i = _sub69i + _mul76i;
    { int _idx = 19; if (_idx >= 0 && _idx < 50) { cRe[_idx] = _add77r; cIm[_idx] = _add77i; } }
    double _abs78r = 0, _abs78i = 0;
    _abs78r = c_abs(x1r, x1i); _abs78i = 0;
    double _abs79r = 0, _abs79i = 0;
    _abs79r = c_abs(x2r, x2i); _abs79i = 0;
    double _add80r = 0, _add80i = 0;
    _add80r = _abs78r + _abs79r; _add80i = _abs78i + _abs79i;
    double _c81r = 0, _c81i = 0;
    _c81r = 1.0; _c81i = 0;
    double _add82r = 0, _add82i = 0;
    _add82r = _add80r + _c81r; _add82i = _add80i + _c81i;
    double _log83r = 0, _log83i = 0;
    c_log(_add82r, _add82i, &_log83r, &_log83i);
    double _ang84r = 0, _ang84i = 0;
    _ang84r = c_arg(x1r, x1i); _ang84i = 0;
    double _sin85r = 0, _sin85i = 0;
    c_sin(_ang84r, _ang84i, &_sin85r, &_sin85i);
    double _ang86r = 0, _ang86i = 0;
    _ang86r = c_arg(x2r, x2i); _ang86i = 0;
    double _cos87r = 0, _cos87i = 0;
    c_cos(_ang86r, _ang86i, &_cos87r, &_cos87i);
    double _c88r = 0, _c88i = 0;
    _c88r = 0.0; _c88i = 1.0;
    double _mul89r = 0, _mul89i = 0;
    c_mul(_cos87r, _cos87i, _c88r, _c88i, &_mul89r, &_mul89i);
    double _add90r = 0, _add90i = 0;
    _add90r = _sin85r + _mul89r; _add90i = _sin85i + _mul89i;
    double _mul91r = 0, _mul91i = 0;
    c_mul(_log83r, _log83i, _add90r, _add90i, &_mul91r, &_mul91i);
    { int _idx = 29; if (_idx >= 0 && _idx < 50) { cRe[_idx] = _mul91r; cIm[_idx] = _mul91i; } }
    double _abs92r = 0, _abs92i = 0;
    _abs92r = c_abs(x1r, x1i); _abs92i = 0;
    double _abs93r = 0, _abs93i = 0;
    _abs93r = c_abs(x2r, x2i); _abs93i = 0;
    double _mul94r = 0, _mul94i = 0;
    c_mul(_abs92r, _abs92i, _abs93r, _abs93i, &_mul94r, &_mul94i);
    double _c95r = 0, _c95i = 0;
    _c95r = 0.0; _c95i = 1.0;
    double _ang96r = 0, _ang96i = 0;
    _ang96r = c_arg(x1r, x1i); _ang96i = 0;
    double _ang97r = 0, _ang97i = 0;
    _ang97r = c_arg(x2r, x2i); _ang97i = 0;
    double _sub98r = 0, _sub98i = 0;
    _sub98r = _ang96r - _ang97r; _sub98i = _ang96i - _ang97i;
    double _mul99r = 0, _mul99i = 0;
    c_mul(_c95r, _c95i, _sub98r, _sub98i, &_mul99r, &_mul99i);
    double _exp100r = 0, _exp100i = 0;
    c_exp2(_mul99r, _mul99i, &_exp100r, &_exp100i);
    double _mul101r = 0, _mul101i = 0;
    c_mul(_mul94r, _mul94i, _exp100r, _exp100i, &_mul101r, &_mul101i);
    { int _idx = 39; if (_idx >= 0 && _idx < 50) { cRe[_idx] = _mul101r; cIm[_idx] = _mul101i; } }
    double _conj102r = 0, _conj102i = 0;
    _conj102r = x1r; _conj102i = -(x1i);
    double _conj103r = 0, _conj103i = 0;
    _conj103r = x2r; _conj103i = -(x2i);
    double _add104r = 0, _add104i = 0;
    _add104r = _conj102r + _conj103r; _add104i = _conj102i + _conj103i;
    double _mul105r = 0, _mul105i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul105r, &_mul105i);
    double _c106r = 0, _c106i = 0;
    _c106r = 0.0; _c106i = 1.0;
    double _mul107r = 0, _mul107i = 0;
    c_mul(_mul105r, _mul105i, _c106r, _c106i, &_mul107r, &_mul107i);
    double _sub108r = 0, _sub108i = 0;
    _sub108r = _add104r - _mul107r; _sub108i = _add104i - _mul107i;
    { int _idx = 49; if (_idx >= 0 && _idx < 50) { cRe[_idx] = _sub108r; cIm[_idx] = _sub108i; } }
    for (int _i = 0; _i < 50; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_232_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
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
        double k_r = _add4r, k_i = _add4i;
        double _c5r = 0, _c5i = 0;
        _c5r = 7.0; _c5i = 0;
        double _fdiv6r = 0, _fdiv6i = 0;
        c_div(j, 0, _c5r, _c5i, &_fdiv6r, &_fdiv6i);
        _fdiv6r = floor(_fdiv6r); _fdiv6i = 0;
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _fdiv6r + _c7r; _add8i = _fdiv6i + _c7i;
        double r_r = _add8r, r_i = _add8i;
        double _re9r = 0, _re9i = 0;
        _re9r = x1r; _re9i = 0;
        double _pow10r = 0, _pow10i = 0;
        c_powc(_re9r, _re9i, k_r, k_i, &_pow10r, &_pow10i);
        double _im11r = 0, _im11i = 0;
        _im11r = x2i; _im11i = 0;
        double _pow12r = 0, _pow12i = 0;
        c_powc(_im11r, _im11i, r_r, r_i, &_pow12r, &_pow12i);
        double _sub13r = 0, _sub13i = 0;
        _sub13r = _pow10r - _pow12r; _sub13i = _pow10i - _pow12i;
        double _add14r = 0, _add14i = 0;
        _add14r = x1r + x2r; _add14i = x1i + x2i;
        double _ang15r = 0, _ang15i = 0;
        _ang15r = c_arg(_add14r, _add14i); _ang15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(j, 0, _ang15r, _ang15i, &_mul16r, &_mul16i);
        double _sin17r = 0, _sin17i = 0;
        c_sin(_mul16r, _mul16i, &_sin17r, &_sin17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_sub13r, _sub13i, _sin17r, _sin17i, &_mul18r, &_mul18i);
        double _abs19r = 0, _abs19i = 0;
        _abs19r = c_abs(x1r, x1i); _abs19i = 0;
        double _abs20r = 0, _abs20i = 0;
        _abs20r = c_abs(x2r, x2i); _abs20i = 0;
        double _add21r = 0, _add21i = 0;
        _add21r = _abs19r + _abs20r; _add21i = _abs19i + _abs20i;
        double _c22r = 0, _c22i = 0;
        _c22r = 1.0; _c22i = 0;
        double _add23r = 0, _add23i = 0;
        _add23r = _add21r + _c22r; _add23i = _add21i + _c22i;
        double _div24r = 0, _div24i = 0;
        c_div(_mul18r, _mul18i, _add23r, _add23i, &_div24r, &_div24i);
        double _conj25r = 0, _conj25i = 0;
        _conj25r = x1r; _conj25i = -(x1i);
        double _pow26r = 0, _pow26i = 0;
        c_powc(_conj25r, _conj25i, k_r, k_i, &_pow26r, &_pow26i);
        double _ang27r = 0, _ang27i = 0;
        _ang27r = c_arg(x2r, x2i); _ang27i = 0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(r_r, r_i, _ang27r, _ang27i, &_mul28r, &_mul28i);
        double _cos29r = 0, _cos29i = 0;
        c_cos(_mul28r, _mul28i, &_cos29r, &_cos29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_pow26r, _pow26i, _cos29r, _cos29i, &_mul30r, &_mul30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _div24r + _mul30r; _add31i = _div24i + _mul30i;
        double _abs32r = 0, _abs32i = 0;
        _abs32r = c_abs(x1r, x1i); _abs32i = 0;
        double _c33r = 0, _c33i = 0;
        _c33r = 1.0; _c33i = 0;
        double _add34r = 0, _add34i = 0;
        _add34r = _abs32r + _c33r; _add34i = _abs32i + _c33i;
        double _log35r = 0, _log35i = 0;
        c_log(_add34r, _add34i, &_log35r, &_log35i);
        double _abs36r = 0, _abs36i = 0;
        _abs36r = c_abs(x2r, x2i); _abs36i = 0;
        double _c37r = 0, _c37i = 0;
        _c37r = 1.0; _c37i = 0;
        double _add38r = 0, _add38i = 0;
        _add38r = _abs36r + _c37r; _add38i = _abs36i + _c37i;
        double _log39r = 0, _log39i = 0;
        c_log(_add38r, _add38i, &_log39r, &_log39i);
        double _mul40r = 0, _mul40i = 0;
        c_mul(_log35r, _log35i, _log39r, _log39i, &_mul40r, &_mul40i);
        double _add41r = 0, _add41i = 0;
        _add41r = _add31r + _mul40r; _add41i = _add31i + _mul40i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add41r; cIm[_idx] = _add41i; } }
    }
    double _mul42r = 0, _mul42i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul42r, &_mul42i);
    double _c43r = 0, _c43i = 0;
    _c43r = 2.0; _c43i = 0;
    double _pow44r = 0, _pow44i = 0;
    c_mul(_mul42r, _mul42i, _mul42r, _mul42i, &_pow44r, &_pow44i);
    double _conj45r = 0, _conj45i = 0;
    _conj45r = x1r; _conj45i = -(x1i);
    double _sin46r = 0, _sin46i = 0;
    c_sin(x2r, x2i, &_sin46r, &_sin46i);
    double _mul47r = 0, _mul47i = 0;
    c_mul(_conj45r, _conj45i, _sin46r, _sin46i, &_mul47r, &_mul47i);
    double _sub48r = 0, _sub48i = 0;
    _sub48r = _pow44r - _mul47r; _sub48i = _pow44i - _mul47i;
    { int _idx = 2; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub48r; cIm[_idx] = _sub48i; } }
    double _abs49r = 0, _abs49i = 0;
    _abs49r = c_abs(x1r, x1i); _abs49i = 0;
    double _re50r = 0, _re50i = 0;
    _re50r = x2r; _re50i = 0;
    double _mul51r = 0, _mul51i = 0;
    c_mul(_abs49r, _abs49i, _re50r, _re50i, &_mul51r, &_mul51i);
    double _im52r = 0, _im52i = 0;
    _im52r = x1i; _im52i = 0;
    double _im53r = 0, _im53i = 0;
    _im53r = x2i; _im53i = 0;
    double _mul54r = 0, _mul54i = 0;
    c_mul(_im52r, _im52i, _im53r, _im53i, &_mul54r, &_mul54i);
    double _add55r = 0, _add55i = 0;
    _add55r = _mul51r + _mul54r; _add55i = _mul51i + _mul54i;
    { int _idx = 7; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add55r; cIm[_idx] = _add55i; } }
    double _cos56r = 0, _cos56i = 0;
    c_cos(x1r, x1i, &_cos56r, &_cos56i);
    double _sin57r = 0, _sin57i = 0;
    c_sin(x2r, x2i, &_sin57r, &_sin57i);
    double _add58r = 0, _add58i = 0;
    _add58r = _cos56r + _sin57r; _add58i = _cos56i + _sin57i;
    { int _idx = 11; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add58r; cIm[_idx] = _add58i; } }
    double _abs59r = 0, _abs59i = 0;
    _abs59r = c_abs(x1r, x1i); _abs59i = 0;
    double _c60r = 0, _c60i = 0;
    _c60r = 1.0; _c60i = 0;
    double _add61r = 0, _add61i = 0;
    _add61r = _abs59r + _c60r; _add61i = _abs59i + _c60i;
    double _log62r = 0, _log62i = 0;
    c_log(_add61r, _add61i, &_log62r, &_log62i);
    double _abs63r = 0, _abs63i = 0;
    _abs63r = c_abs(x2r, x2i); _abs63i = 0;
    double _c64r = 0, _c64i = 0;
    _c64r = 1.0; _c64i = 0;
    double _add65r = 0, _add65i = 0;
    _add65r = _abs63r + _c64r; _add65i = _abs63i + _c64i;
    double _log66r = 0, _log66i = 0;
    c_log(_add65r, _add65i, &_log66r, &_log66i);
    double _add67r = 0, _add67i = 0;
    _add67r = _log62r + _log66r; _add67i = _log62i + _log66i;
    { int _idx = 18; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add67r; cIm[_idx] = _add67i; } }
    double _c68r = 0, _c68i = 0;
    _c68r = 3.0; _c68i = 0;
    double _pow69r = 0, _pow69i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow69r, &_pow69i);
    c_mul(_pow69r, _pow69i, x1r, x1i, &_pow69r, &_pow69i);
    double _c70r = 0, _c70i = 0;
    _c70r = 3.0; _c70i = 0;
    double _pow71r = 0, _pow71i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow71r, &_pow71i);
    c_mul(_pow71r, _pow71i, x2r, x2i, &_pow71r, &_pow71i);
    double _sub72r = 0, _sub72i = 0;
    _sub72r = _pow69r - _pow71r; _sub72i = _pow69i - _pow71i;
    double _mul73r = 0, _mul73i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul73r, &_mul73i);
    double _conj74r = 0, _conj74i = 0;
    _conj74r = _mul73r; _conj74i = -(_mul73i);
    double _add75r = 0, _add75i = 0;
    _add75r = _sub72r + _conj74r; _add75i = _sub72i + _conj74i;
    { int _idx = 22; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add75r; cIm[_idx] = _add75i; } }
    double _add76r = 0, _add76i = 0;
    _add76r = x1r + x2r; _add76i = x1i + x2i;
    double _re77r = 0, _re77i = 0;
    _re77r = _add76r; _re77i = 0;
    double _sub78r = 0, _sub78i = 0;
    _sub78r = x1r - x2r; _sub78i = x1i - x2i;
    double _im79r = 0, _im79i = 0;
    _im79r = _sub78i; _im79i = 0;
    double _mul80r = 0, _mul80i = 0;
    c_mul(_re77r, _re77i, _im79r, _im79i, &_mul80r, &_mul80i);
    { int _idx = 28; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul80r; cIm[_idx] = _mul80i; } }
    double _mul81r = 0, _mul81i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul81r, &_mul81i);
    double _sin82r = 0, _sin82i = 0;
    c_sin(_mul81r, _mul81i, &_sin82r, &_sin82i);
    double _conj83r = 0, _conj83i = 0;
    _conj83r = x1r; _conj83i = -(x1i);
    double _cos84r = 0, _cos84i = 0;
    c_cos(_conj83r, _conj83i, &_cos84r, &_cos84i);
    double _conj85r = 0, _conj85i = 0;
    _conj85r = x2r; _conj85i = -(x2i);
    double _cos86r = 0, _cos86i = 0;
    c_cos(_conj85r, _conj85i, &_cos86r, &_cos86i);
    double _mul87r = 0, _mul87i = 0;
    c_mul(_cos84r, _cos84i, _cos86r, _cos86i, &_mul87r, &_mul87i);
    double _add88r = 0, _add88i = 0;
    _add88r = _sin82r + _mul87r; _add88i = _sin82i + _mul87i;
    { int _idx = 33; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add88r; cIm[_idx] = _add88i; } }
    double _re89r = 0, _re89i = 0;
    _re89r = x1r; _re89i = 0;
    double _c90r = 0, _c90i = 0;
    _c90r = 2.0; _c90i = 0;
    double _pow91r = 0, _pow91i = 0;
    c_mul(_re89r, _re89i, _re89r, _re89i, &_pow91r, &_pow91i);
    double _im92r = 0, _im92i = 0;
    _im92r = x2i; _im92i = 0;
    double _c93r = 0, _c93i = 0;
    _c93r = 2.0; _c93i = 0;
    double _pow94r = 0, _pow94i = 0;
    c_mul(_im92r, _im92i, _im92r, _im92i, &_pow94r, &_pow94i);
    double _add95r = 0, _add95i = 0;
    _add95r = _pow91r + _pow94r; _add95i = _pow91i + _pow94i;
    { int _idx = 4; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add95r; cIm[_idx] = _add95i; } }
    double _ang96r = 0, _ang96i = 0;
    _ang96r = c_arg(x1r, x1i); _ang96i = 0;
    double _ang97r = 0, _ang97i = 0;
    _ang97r = c_arg(x2r, x2i); _ang97i = 0;
    double _mul98r = 0, _mul98i = 0;
    c_mul(_ang96r, _ang96i, _ang97r, _ang97i, &_mul98r, &_mul98i);
    { int _idx = 9; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul98r; cIm[_idx] = _mul98i; } }
    double _add99r = 0, _add99i = 0;
    _add99r = x1r + x2r; _add99i = x1i + x2i;
    double _abs100r = 0, _abs100i = 0;
    _abs100r = c_abs(_add99r, _add99i); _abs100i = 0;
    double _sub101r = 0, _sub101i = 0;
    _sub101r = x1r - x2r; _sub101i = x1i - x2i;
    double _conj102r = 0, _conj102i = 0;
    _conj102r = _sub101r; _conj102i = -(_sub101i);
    double _mul103r = 0, _mul103i = 0;
    c_mul(_abs100r, _abs100i, _conj102r, _conj102i, &_mul103r, &_mul103i);
    { int _idx = 14; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul103r; cIm[_idx] = _mul103i; } }
    double _abs104r = 0, _abs104i = 0;
    _abs104r = c_abs(x1r, x1i); _abs104i = 0;
    double _sin105r = 0, _sin105i = 0;
    c_sin(_abs104r, _abs104i, &_sin105r, &_sin105i);
    double _abs106r = 0, _abs106i = 0;
    _abs106r = c_abs(x2r, x2i); _abs106i = 0;
    double _cos107r = 0, _cos107i = 0;
    c_cos(_abs106r, _abs106i, &_cos107r, &_cos107i);
    double _mul108r = 0, _mul108i = 0;
    c_mul(_sin105r, _sin105i, _cos107r, _cos107i, &_mul108r, &_mul108i);
    { int _idx = 19; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul108r; cIm[_idx] = _mul108i; } }
    double _mul109r = 0, _mul109i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul109r, &_mul109i);
    double _abs110r = 0, _abs110i = 0;
    _abs110r = c_abs(_mul109r, _mul109i); _abs110i = 0;
    double _c111r = 0, _c111i = 0;
    _c111r = 1.0; _c111i = 0;
    double _add112r = 0, _add112i = 0;
    _add112r = _abs110r + _c111r; _add112i = _abs110i + _c111i;
    double _log113r = 0, _log113i = 0;
    c_log(_add112r, _add112i, &_log113r, &_log113i);
    { int _idx = 24; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _log113r; cIm[_idx] = _log113i; } }
    double _re114r = 0, _re114i = 0;
    _re114r = x1r; _re114i = 0;
    double _im115r = 0, _im115i = 0;
    _im115r = x2i; _im115i = 0;
    double _sum116r = 0, _sum116i = 0;
    _sum116r = _re114r + _im115r; _sum116i = _re114i + _im115i;
    double _add117r = 0, _add117i = 0;
    _add117r = x1r + x2r; _add117i = x1i + x2i;
    double _ang118r = 0, _ang118i = 0;
    _ang118r = c_arg(_add117r, _add117i); _ang118i = 0;
    double _sum119r = 0, _sum119i = 0;
    _sum119r = _sum116r + _ang118r; _sum119i = _sum116i + _ang118i;
    { int _idx = 29; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sum119r; cIm[_idx] = _sum119i; } }
    double _c120r = 0, _c120i = 0;
    _c120r = 2.0; _c120i = 0;
    double _pow121r = 0, _pow121i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow121r, &_pow121i);
    double _mul122r = 0, _mul122i = 0;
    c_mul(_pow121r, _pow121i, x2r, x2i, &_mul122r, &_mul122i);
    double _c123r = 0, _c123i = 0;
    _c123r = 2.0; _c123i = 0;
    double _pow124r = 0, _pow124i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow124r, &_pow124i);
    double _mul125r = 0, _mul125i = 0;
    c_mul(x1r, x1i, _pow124r, _pow124i, &_mul125r, &_mul125i);
    double _sub126r = 0, _sub126i = 0;
    _sub126r = _mul122r - _mul125r; _sub126i = _mul122i - _mul125i;
    { int _idx = 6; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub126r; cIm[_idx] = _sub126i; } }
    double _conj127r = 0, _conj127i = 0;
    _conj127r = x1r; _conj127i = -(x1i);
    double _c128r = 0, _c128i = 0;
    _c128r = 2.0; _c128i = 0;
    double _pow129r = 0, _pow129i = 0;
    c_mul(_conj127r, _conj127i, _conj127r, _conj127i, &_pow129r, &_pow129i);
    double _conj130r = 0, _conj130i = 0;
    _conj130r = x2r; _conj130i = -(x2i);
    double _c131r = 0, _c131i = 0;
    _c131r = 2.0; _c131i = 0;
    double _pow132r = 0, _pow132i = 0;
    c_mul(_conj130r, _conj130i, _conj130r, _conj130i, &_pow132r, &_pow132i);
    double _add133r = 0, _add133i = 0;
    _add133r = _pow129r + _pow132r; _add133i = _pow129i + _pow132i;
    { int _idx = 10; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add133r; cIm[_idx] = _add133i; } }
    double _sin134r = 0, _sin134i = 0;
    c_sin(x1r, x1i, &_sin134r, &_sin134i);
    double _cos135r = 0, _cos135i = 0;
    c_cos(x2r, x2i, &_cos135r, &_cos135i);
    double _mul136r = 0, _mul136i = 0;
    c_mul(_sin134r, _sin134i, _cos135r, _cos135i, &_mul136r, &_mul136i);
    double _cos137r = 0, _cos137i = 0;
    c_cos(x1r, x1i, &_cos137r, &_cos137i);
    double _sin138r = 0, _sin138i = 0;
    c_sin(x2r, x2i, &_sin138r, &_sin138i);
    double _mul139r = 0, _mul139i = 0;
    c_mul(_cos137r, _cos137i, _sin138r, _sin138i, &_mul139r, &_mul139i);
    double _add140r = 0, _add140i = 0;
    _add140r = _mul136r + _mul139r; _add140i = _mul136i + _mul139i;
    { int _idx = 16; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add140r; cIm[_idx] = _add140i; } }
    double _mul141r = 0, _mul141i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul141r, &_mul141i);
    double _re142r = 0, _re142i = 0;
    _re142r = _mul141r; _re142i = 0;
    double _mul143r = 0, _mul143i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul143r, &_mul143i);
    double _im144r = 0, _im144i = 0;
    _im144r = _mul143i; _im144i = 0;
    double _add145r = 0, _add145i = 0;
    _add145r = _re142r + _im144r; _add145i = _re142i + _im144i;
    { int _idx = 20; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add145r; cIm[_idx] = _add145i; } }
    double _add146r = 0, _add146i = 0;
    _add146r = x1r + x2r; _add146i = x1i + x2i;
    double _c147r = 0, _c147i = 0;
    _c147r = 3.0; _c147i = 0;
    double _pow148r = 0, _pow148i = 0;
    c_mul(_add146r, _add146i, _add146r, _add146i, &_pow148r, &_pow148i);
    c_mul(_pow148r, _pow148i, _add146r, _add146i, &_pow148r, &_pow148i);
    double _sub149r = 0, _sub149i = 0;
    _sub149r = x1r - x2r; _sub149i = x1i - x2i;
    double _c150r = 0, _c150i = 0;
    _c150r = 3.0; _c150i = 0;
    double _pow151r = 0, _pow151i = 0;
    c_mul(_sub149r, _sub149i, _sub149r, _sub149i, &_pow151r, &_pow151i);
    c_mul(_pow151r, _pow151i, _sub149r, _sub149i, &_pow151r, &_pow151i);
    double _sub152r = 0, _sub152i = 0;
    _sub152r = _pow148r - _pow151r; _sub152i = _pow148i - _pow151i;
    { int _idx = 26; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub152r; cIm[_idx] = _sub152i; } }
    double _abs153r = 0, _abs153i = 0;
    _abs153r = c_abs(x1r, x1i); _abs153i = 0;
    double _abs154r = 0, _abs154i = 0;
    _abs154r = c_abs(x2r, x2i); _abs154i = 0;
    double _prod155r = 0, _prod155i = 0;
    c_mul(_abs153r, _abs153i, _abs154r, _abs154i, &_prod155r, &_prod155i);
    double _add156r = 0, _add156i = 0;
    _add156r = x1r + x2r; _add156i = x1i + x2i;
    double _re157r = 0, _re157i = 0;
    _re157r = _add156r; _re157i = 0;
    double _prod158r = 0, _prod158i = 0;
    c_mul(_prod155r, _prod155i, _re157r, _re157i, &_prod158r, &_prod158i);
    { int _idx = 30; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _prod158r; cIm[_idx] = _prod158i; } }
    double _re159r = 0, _re159i = 0;
    _re159r = x1r; _re159i = 0;
    double _c160r = 0, _c160i = 0;
    _c160r = 3.0; _c160i = 0;
    double _pow161r = 0, _pow161i = 0;
    c_mul(_re159r, _re159i, _re159r, _re159i, &_pow161r, &_pow161i);
    c_mul(_pow161r, _pow161i, _re159r, _re159i, &_pow161r, &_pow161i);
    double _im162r = 0, _im162i = 0;
    _im162r = x2i; _im162i = 0;
    double _c163r = 0, _c163i = 0;
    _c163r = 3.0; _c163i = 0;
    double _pow164r = 0, _pow164i = 0;
    c_mul(_im162r, _im162i, _im162r, _im162i, &_pow164r, &_pow164i);
    c_mul(_pow164r, _pow164i, _im162r, _im162i, &_pow164r, &_pow164i);
    double _sub165r = 0, _sub165i = 0;
    _sub165r = _pow161r - _pow164r; _sub165i = _pow161i - _pow164i;
    { int _idx = 8; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub165r; cIm[_idx] = _sub165i; } }
    double _ang166r = 0, _ang166i = 0;
    _ang166r = c_arg(x1r, x1i); _ang166i = 0;
    double _c167r = 0, _c167i = 0;
    _c167r = 2.0; _c167i = 0;
    double _pow168r = 0, _pow168i = 0;
    c_mul(_ang166r, _ang166i, _ang166r, _ang166i, &_pow168r, &_pow168i);
    double _ang169r = 0, _ang169i = 0;
    _ang169r = c_arg(x2r, x2i); _ang169i = 0;
    double _c170r = 0, _c170i = 0;
    _c170r = 2.0; _c170i = 0;
    double _pow171r = 0, _pow171i = 0;
    c_mul(_ang169r, _ang169i, _ang169r, _ang169i, &_pow171r, &_pow171i);
    double _add172r = 0, _add172i = 0;
    _add172r = _pow168r + _pow171r; _add172i = _pow168i + _pow171i;
    { int _idx = 12; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add172r; cIm[_idx] = _add172i; } }
    double _add173r = 0, _add173i = 0;
    _add173r = x1r + x2r; _add173i = x1i + x2i;
    double _sin174r = 0, _sin174i = 0;
    c_sin(_add173r, _add173i, &_sin174r, &_sin174i);
    double _sub175r = 0, _sub175i = 0;
    _sub175r = x1r - x2r; _sub175i = x1i - x2i;
    double _cos176r = 0, _cos176i = 0;
    c_cos(_sub175r, _sub175i, &_cos176r, &_cos176i);
    double _sub177r = 0, _sub177i = 0;
    _sub177r = _sin174r - _cos176r; _sub177i = _sin174i - _cos176i;
    { int _idx = 17; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub177r; cIm[_idx] = _sub177i; } }
    double _abs178r = 0, _abs178i = 0;
    _abs178r = c_abs(x1r, x1i); _abs178i = 0;
    double _c179r = 0, _c179i = 0;
    _c179r = 2.0; _c179i = 0;
    double _pow180r = 0, _pow180i = 0;
    c_mul(_abs178r, _abs178i, _abs178r, _abs178i, &_pow180r, &_pow180i);
    double _abs181r = 0, _abs181i = 0;
    _abs181r = c_abs(x2r, x2i); _abs181i = 0;
    double _c182r = 0, _c182i = 0;
    _c182r = 2.0; _c182i = 0;
    double _pow183r = 0, _pow183i = 0;
    c_mul(_abs181r, _abs181i, _abs181r, _abs181i, &_pow183r, &_pow183i);
    double _add184r = 0, _add184i = 0;
    _add184r = _pow180r + _pow183r; _add184i = _pow180i + _pow183i;
    double _c185r = 0, _c185i = 0;
    _c185r = 1.0; _c185i = 0;
    double _add186r = 0, _add186i = 0;
    _add186r = _add184r + _c185r; _add186i = _add184i + _c185i;
    double _log187r = 0, _log187i = 0;
    c_log(_add186r, _add186i, &_log187r, &_log187i);
    { int _idx = 23; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _log187r; cIm[_idx] = _log187i; } }
    double _conj188r = 0, _conj188i = 0;
    _conj188r = x1r; _conj188i = -(x1i);
    double _mul189r = 0, _mul189i = 0;
    c_mul(_conj188r, _conj188i, x2r, x2i, &_mul189r, &_mul189i);
    double _re190r = 0, _re190i = 0;
    _re190r = _mul189r; _re190i = 0;
    { int _idx = 27; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _re190r; cIm[_idx] = _re190i; } }
    double _conj191r = 0, _conj191i = 0;
    _conj191r = x2r; _conj191i = -(x2i);
    double _mul192r = 0, _mul192i = 0;
    c_mul(x1r, x1i, _conj191r, _conj191i, &_mul192r, &_mul192i);
    double _im193r = 0, _im193i = 0;
    _im193r = _mul192i; _im193i = 0;
    { int _idx = 31; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _im193r; cIm[_idx] = _im193i; } }
    double _re194r = 0, _re194i = 0;
    _re194r = x1r; _re194i = 0;
    double _re195r = 0, _re195i = 0;
    _re195r = x2r; _re195i = 0;
    double _mul196r = 0, _mul196i = 0;
    c_mul(_re194r, _re194i, _re195r, _re195i, &_mul196r, &_mul196i);
    { int _idx = 3; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul196r; cIm[_idx] = _mul196i; } }
    double _im197r = 0, _im197i = 0;
    _im197r = x1i; _im197i = 0;
    double _im198r = 0, _im198i = 0;
    _im198r = x2i; _im198i = 0;
    double _mul199r = 0, _mul199i = 0;
    c_mul(_im197r, _im197i, _im198r, _im198i, &_mul199r, &_mul199i);
    { int _idx = 5; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul199r; cIm[_idx] = _mul199i; } }
    double _add200r = 0, _add200i = 0;
    _add200r = x1r + x2r; _add200i = x1i + x2i;
    double _ang201r = 0, _ang201i = 0;
    _ang201r = c_arg(_add200r, _add200i); _ang201i = 0;
    double _mul202r = 0, _mul202i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul202r, &_mul202i);
    double _abs203r = 0, _abs203i = 0;
    _abs203r = c_abs(_mul202r, _mul202i); _abs203i = 0;
    double _mul204r = 0, _mul204i = 0;
    c_mul(_ang201r, _ang201i, _abs203r, _abs203i, &_mul204r, &_mul204i);
    { int _idx = 15; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul204r; cIm[_idx] = _mul204i; } }
    double _re205r = 0, _re205i = 0;
    _re205r = x1r; _re205i = 0;
    double _sin206r = 0, _sin206i = 0;
    c_sin(_re205r, _re205i, &_sin206r, &_sin206i);
    double _im207r = 0, _im207i = 0;
    _im207r = x2i; _im207i = 0;
    double _cos208r = 0, _cos208i = 0;
    c_cos(_im207r, _im207i, &_cos208r, &_cos208i);
    double _add209r = 0, _add209i = 0;
    _add209r = _sin206r + _cos208r; _add209i = _sin206i + _cos208i;
    { int _idx = 20; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add209r; cIm[_idx] = _add209i; } }
    double _add210r = 0, _add210i = 0;
    _add210r = x1r + x2r; _add210i = x1i + x2i;
    double _abs211r = 0, _abs211i = 0;
    _abs211r = c_abs(_add210r, _add210i); _abs211i = 0;
    double _c212r = 0, _c212i = 0;
    _c212r = 1.0; _c212i = 0;
    double _add213r = 0, _add213i = 0;
    _add213r = _abs211r + _c212r; _add213i = _abs211i + _c212i;
    double _log214r = 0, _log214i = 0;
    c_log(_add213r, _add213i, &_log214r, &_log214i);
    { int _idx = 21; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _log214r; cIm[_idx] = _log214i; } }
    double _add215r = 0, _add215i = 0;
    _add215r = x1r + x2r; _add215i = x1i + x2i;
    double _conj216r = 0, _conj216i = 0;
    _conj216r = _add215r; _conj216i = -(_add215i);
    double _re217r = 0, _re217i = 0;
    _re217r = _conj216r; _re217i = 0;
    { int _idx = 25; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _re217r; cIm[_idx] = _re217i; } }
    double _abs218r = 0, _abs218i = 0;
    _abs218r = c_abs(x1r, x1i); _abs218i = 0;
    double _c219r = 0, _c219i = 0;
    _c219r = 2.0; _c219i = 0;
    double _pow220r = 0, _pow220i = 0;
    c_mul(_abs218r, _abs218i, _abs218r, _abs218i, &_pow220r, &_pow220i);
    double _sin221r = 0, _sin221i = 0;
    c_sin(_pow220r, _pow220i, &_sin221r, &_sin221i);
    double _abs222r = 0, _abs222i = 0;
    _abs222r = c_abs(x2r, x2i); _abs222i = 0;
    double _c223r = 0, _c223i = 0;
    _c223r = 2.0; _c223i = 0;
    double _pow224r = 0, _pow224i = 0;
    c_mul(_abs222r, _abs222i, _abs222r, _abs222i, &_pow224r, &_pow224i);
    double _cos225r = 0, _cos225i = 0;
    c_cos(_pow224r, _pow224i, &_cos225r, &_cos225i);
    double _mul226r = 0, _mul226i = 0;
    c_mul(_sin221r, _sin221i, _cos225r, _cos225i, &_mul226r, &_mul226i);
    { int _idx = 32; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul226r; cIm[_idx] = _mul226i; } }
    double _re227r = 0, _re227i = 0;
    _re227r = x1r; _re227i = 0;
    double _c228r = 0, _c228i = 0;
    _c228r = 2.0; _c228i = 0;
    double _pow229r = 0, _pow229i = 0;
    c_mul(_re227r, _re227i, _re227r, _re227i, &_pow229r, &_pow229i);
    double _im230r = 0, _im230i = 0;
    _im230r = x1i; _im230i = 0;
    double _c231r = 0, _c231i = 0;
    _c231r = 2.0; _c231i = 0;
    double _pow232r = 0, _pow232i = 0;
    c_mul(_im230r, _im230i, _im230r, _im230i, &_pow232r, &_pow232i);
    double _add233r = 0, _add233i = 0;
    _add233r = _pow229r + _pow232r; _add233i = _pow229i + _pow232i;
    double _re234r = 0, _re234i = 0;
    _re234r = x2r; _re234i = 0;
    double _c235r = 0, _c235i = 0;
    _c235r = 2.0; _c235i = 0;
    double _pow236r = 0, _pow236i = 0;
    c_mul(_re234r, _re234i, _re234r, _re234i, &_pow236r, &_pow236i);
    double _add237r = 0, _add237i = 0;
    _add237r = _add233r + _pow236r; _add237i = _add233i + _pow236i;
    double _im238r = 0, _im238i = 0;
    _im238r = x2i; _im238i = 0;
    double _c239r = 0, _c239i = 0;
    _c239r = 2.0; _c239i = 0;
    double _pow240r = 0, _pow240i = 0;
    c_mul(_im238r, _im238i, _im238r, _im238i, &_pow240r, &_pow240i);
    double _add241r = 0, _add241i = 0;
    _add241r = _add237r + _pow240r; _add241i = _add237i + _pow240i;
    { int _idx = 34; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add241r; cIm[_idx] = _add241i; } }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_233_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 2.0; _c1i = 0;
    double _c2r = 0, _c2i = 0;
    _c2r = 0.0; _c2i = 3.0;
    double _sub3r = 0, _sub3i = 0;
    _sub3r = _c1r - _c2r; _sub3i = _c1i - _c2i;
    { int _idx = 2; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub3r; cIm[_idx] = _sub3i; } }
    double _c4r = 0, _c4i = 0;
    _c4r = 4.0; _c4i = 0;
    double _neg5r = 0, _neg5i = 0;
    _neg5r = -(_c4r); _neg5i = -(_c4i);
    double _c6r = 0, _c6i = 0;
    _c6r = 0.0; _c6i = 2.0;
    double _add7r = 0, _add7i = 0;
    _add7r = _neg5r + _c6r; _add7i = _neg5i + _c6i;
    { int _idx = 5; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add7r; cIm[_idx] = _add7i; } }
    double _c8r = 0, _c8i = 0;
    _c8r = 5.0; _c8i = 0;
    double _c9r = 0, _c9i = 0;
    _c9r = 0.0; _c9i = 1.0;
    double _sub10r = 0, _sub10i = 0;
    _sub10r = _c8r - _c9r; _sub10i = _c8i - _c9i;
    { int _idx = 11; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub10r; cIm[_idx] = _sub10i; } }
    double _c11r = 0, _c11i = 0;
    _c11r = 6.0; _c11i = 0;
    double _neg12r = 0, _neg12i = 0;
    _neg12r = -(_c11r); _neg12i = -(_c11i);
    double _c13r = 0, _c13i = 0;
    _c13r = 0.0; _c13i = 3.0;
    double _add14r = 0, _add14i = 0;
    _add14r = _neg12r + _c13r; _add14i = _neg12i + _c13i;
    { int _idx = 17; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add14r; cIm[_idx] = _add14i; } }
    double _c15r = 0, _c15i = 0;
    _c15r = 7.0; _c15i = 0;
    double _c16r = 0, _c16i = 0;
    _c16r = 0.0; _c16i = 2.0;
    double _sub17r = 0, _sub17i = 0;
    _sub17r = _c15r - _c16r; _sub17i = _c15i - _c16i;
    { int _idx = 23; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub17r; cIm[_idx] = _sub17i; } }
    double _c18r = 0, _c18i = 0;
    _c18r = 8.0; _c18i = 0;
    double _neg19r = 0, _neg19i = 0;
    _neg19r = -(_c18r); _neg19i = -(_c18i);
    double _c20r = 0, _c20i = 0;
    _c20r = 0.0; _c20i = 4.0;
    double _add21r = 0, _add21i = 0;
    _add21r = _neg19r + _c20r; _add21i = _neg19i + _c20i;
    { int _idx = 29; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add21r; cIm[_idx] = _add21i; } }
    for (int j = 1; j < 36; j++) {
        double _c22r = 0, _c22i = 0;
        _c22r = 4.0; _c22i = 0;
        double _mod23r = 0, _mod23i = 0;
        _mod23r = fmod(j, _c22r); _mod23i = 0;
        double _c24r = 0, _c24i = 0;
        _c24r = 0.0; _c24i = 0;
        if (_mod23r == _c24r) {
            double _re25r = 0, _re25i = 0;
            _re25r = x1r; _re25i = 0;
            double _c26r = 0, _c26i = 0;
            _c26r = 2.0; _c26i = 0;
            double _pow27r = 0, _pow27i = 0;
            c_mul(j, 0, j, 0, &_pow27r, &_pow27i);
            double _mul28r = 0, _mul28i = 0;
            c_mul(_re25r, _re25i, _pow27r, _pow27i, &_mul28r, &_mul28i);
            double _im29r = 0, _im29i = 0;
            _im29r = x2i; _im29i = 0;
            double _mul30r = 0, _mul30i = 0;
            c_mul(_im29r, _im29i, j, 0, &_mul30r, &_mul30i);
            double _sub31r = 0, _sub31i = 0;
            _sub31r = _mul28r - _mul30r; _sub31i = _mul28i - _mul30i;
            double _im32r = 0, _im32i = 0;
            _im32r = x1i; _im32i = 0;
            double _re33r = 0, _re33i = 0;
            _re33r = x2r; _re33i = 0;
            double _add34r = 0, _add34i = 0;
            _add34r = _im32r + _re33r; _add34i = _im32i + _re33i;
            double _c35r = 0, _c35i = 0;
            _c35r = 0.0; _c35i = 1.0;
            double _mul36r = 0, _mul36i = 0;
            c_mul(_add34r, _add34i, _c35r, _c35i, &_mul36r, &_mul36i);
            double _add37r = 0, _add37i = 0;
            _add37r = _sub31r + _mul36r; _add37i = _sub31i + _mul36i;
            { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add37r; cIm[_idx] = _add37i; } }
        } else {
            double _c38r = 0, _c38i = 0;
            _c38r = 5.0; _c38i = 0;
            double _mod39r = 0, _mod39i = 0;
            _mod39r = fmod(j, _c38r); _mod39i = 0;
            double _c40r = 0, _c40i = 0;
            _c40r = 0.0; _c40i = 0;
            if (_mod39r == _c40r) {
                double _abs41r = 0, _abs41i = 0;
                _abs41r = c_abs(x1r, x1i); _abs41i = 0;
                double _c42r = 0, _c42i = 0;
                _c42r = 3.0; _c42i = 0;
                double _mod43r = 0, _mod43i = 0;
                _mod43r = fmod(j, _c42r); _mod43i = 0;
                double _c44r = 0, _c44i = 0;
                _c44r = 1.0; _c44i = 0;
                double _add45r = 0, _add45i = 0;
                _add45r = _mod43r + _c44r; _add45i = _mod43i + _c44i;
                double _pow46r = 0, _pow46i = 0;
                c_powc(_abs41r, _abs41i, _add45r, _add45i, &_pow46r, &_pow46i);
                double _ang47r = 0, _ang47i = 0;
                _ang47r = c_arg(x2r, x2i); _ang47i = 0;
                double _mul48r = 0, _mul48i = 0;
                c_mul(_ang47r, _ang47i, j, 0, &_mul48r, &_mul48i);
                double _cos49r = 0, _cos49i = 0;
                c_cos(_mul48r, _mul48i, &_cos49r, &_cos49i);
                double _mul50r = 0, _mul50i = 0;
                c_mul(_pow46r, _pow46i, _cos49r, _cos49i, &_mul50r, &_mul50i);
                double _ang51r = 0, _ang51i = 0;
                _ang51r = c_arg(x1r, x1i); _ang51i = 0;
                double _mul52r = 0, _mul52i = 0;
                c_mul(_ang51r, _ang51i, j, 0, &_mul52r, &_mul52i);
                double _sin53r = 0, _sin53i = 0;
                c_sin(_mul52r, _mul52i, &_sin53r, &_sin53i);
                double _abs54r = 0, _abs54i = 0;
                _abs54r = c_abs(x2r, x2i); _abs54i = 0;
                double _mul55r = 0, _mul55i = 0;
                c_mul(_sin53r, _sin53i, _abs54r, _abs54i, &_mul55r, &_mul55i);
                double _c56r = 0, _c56i = 0;
                _c56r = 0.0; _c56i = 1.0;
                double _mul57r = 0, _mul57i = 0;
                c_mul(_mul55r, _mul55i, _c56r, _c56i, &_mul57r, &_mul57i);
                double _add58r = 0, _add58i = 0;
                _add58r = _mul50r + _mul57r; _add58i = _mul50i + _mul57i;
                { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add58r; cIm[_idx] = _add58i; } }
            } else {
                double _c59r = 0, _c59i = 0;
                _c59r = 3.0; _c59i = 0;
                double _mod60r = 0, _mod60i = 0;
                _mod60r = fmod(j, _c59r); _mod60i = 0;
                double _c61r = 0, _c61i = 0;
                _c61r = 0.0; _c61i = 0;
                if (_mod60r == _c61r) {
                    double _abs62r = 0, _abs62i = 0;
                    _abs62r = c_abs(x1r, x1i); _abs62i = 0;
                    double _c63r = 0, _c63i = 0;
                    _c63r = 1.0; _c63i = 0;
                    double _add64r = 0, _add64i = 0;
                    _add64r = _abs62r + _c63r; _add64i = _abs62i + _c63i;
                    double _log65r = 0, _log65i = 0;
                    c_log(_add64r, _add64i, &_log65r, &_log65i);
                    double _mul66r = 0, _mul66i = 0;
                    c_mul(_log65r, _log65i, j, 0, &_mul66r, &_mul66i);
                    double _abs67r = 0, _abs67i = 0;
                    _abs67r = c_abs(x2r, x2i); _abs67i = 0;
                    double _c68r = 0, _c68i = 0;
                    _c68r = 1.0; _c68i = 0;
                    double _add69r = 0, _add69i = 0;
                    _add69r = _abs67r + _c68r; _add69i = _abs67i + _c68i;
                    double _log70r = 0, _log70i = 0;
                    c_log(_add69r, _add69i, &_log70r, &_log70i);
                    double _mul71r = 0, _mul71i = 0;
                    c_mul(_log70r, _log70i, j, 0, &_mul71r, &_mul71i);
                    double _c72r = 0, _c72i = 0;
                    _c72r = 0.0; _c72i = 1.0;
                    double _mul73r = 0, _mul73i = 0;
                    c_mul(_mul71r, _mul71i, _c72r, _c72i, &_mul73r, &_mul73i);
                    double _sub74r = 0, _sub74i = 0;
                    _sub74r = _mul66r - _mul73r; _sub74i = _mul66i - _mul73i;
                    { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub74r; cIm[_idx] = _sub74i; } }
                } else {
                    double _re75r = 0, _re75i = 0;
                    _re75r = x1r; _re75i = 0;
                    double _re76r = 0, _re76i = 0;
                    _re76r = x2r; _re76i = 0;
                    double _add77r = 0, _add77i = 0;
                    _add77r = _re75r + _re76r; _add77i = _re75i + _re76i;
                    double _mul78r = 0, _mul78i = 0;
                    c_mul(_add77r, _add77i, j, 0, &_mul78r, &_mul78i);
                    double _im79r = 0, _im79i = 0;
                    _im79r = x1i; _im79i = 0;
                    double _im80r = 0, _im80i = 0;
                    _im80r = x2i; _im80i = 0;
                    double _sub81r = 0, _sub81i = 0;
                    _sub81r = _im79r - _im80r; _sub81i = _im79i - _im80i;
                    double _mul82r = 0, _mul82i = 0;
                    c_mul(_sub81r, _sub81i, j, 0, &_mul82r, &_mul82i);
                    double _c83r = 0, _c83i = 0;
                    _c83r = 0.0; _c83i = 1.0;
                    double _mul84r = 0, _mul84i = 0;
                    c_mul(_mul82r, _mul82i, _c83r, _c83i, &_mul84r, &_mul84i);
                    double _add85r = 0, _add85i = 0;
                    _add85r = _mul78r + _mul84r; _add85i = _mul78i + _mul84i;
                    { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add85r; cIm[_idx] = _add85i; } }
                }
            }
        }
    }
    for (int k = 1; k < 36; k++) {
        double _c86r = 0, _c86i = 0;
        _c86r = 7.0; _c86i = 0;
        double _mod87r = 0, _mod87i = 0;
        _mod87r = fmod(k, _c86r); _mod87i = 0;
        double _c88r = 0, _c88i = 0;
        _c88r = 0.0; _c88i = 0;
        if (_mod87r == _c88r) {
            double _conj89r = 0, _conj89i = 0;
            _conj89r = x1r; _conj89i = -(x1i);
            double _c90r = 0, _c90i = 0;
            _c90r = 2.0; _c90i = 0;
            double _pow91r = 0, _pow91i = 0;
            c_mul(x2r, x2i, x2r, x2i, &_pow91r, &_pow91i);
            double _mul92r = 0, _mul92i = 0;
            c_mul(_conj89r, _conj89i, _pow91r, _pow91i, &_mul92r, &_mul92i);
            double _sin93r = 0, _sin93i = 0;
            c_sin(x1r, x1i, &_sin93r, &_sin93i);
            double _cos94r = 0, _cos94i = 0;
            c_cos(x2r, x2i, &_cos94r, &_cos94i);
            double _sub95r = 0, _sub95i = 0;
            _sub95r = _sin93r - _cos94r; _sub95i = _sin93i - _cos94i;
            double _c96r = 0, _c96i = 0;
            _c96r = 0.0; _c96i = 1.0;
            double _mul97r = 0, _mul97i = 0;
            c_mul(_sub95r, _sub95i, _c96r, _c96i, &_mul97r, &_mul97i);
            double _add98r = 0, _add98i = 0;
            _add98r = _mul92r + _mul97r; _add98i = _mul92i + _mul97i;
            { int _idx = (k - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add98r; cIm[_idx] = _add98i; } }
        }
        double _c99r = 0, _c99i = 0;
        _c99r = 11.0; _c99i = 0;
        double _mod100r = 0, _mod100i = 0;
        _mod100r = fmod(k, _c99r); _mod100i = 0;
        double _c101r = 0, _c101i = 0;
        _c101r = 0.0; _c101i = 0;
        if (_mod100r == _c101r) {
            double _re102r = 0, _re102i = 0;
            _re102r = x1r; _re102i = 0;
            double _im103r = 0, _im103i = 0;
            _im103r = x2i; _im103i = 0;
            double _prod104r = 0, _prod104i = 0;
            c_mul(_re102r, _re102i, _im103r, _im103i, &_prod104r, &_prod104i);
            double _prod105r = 0, _prod105i = 0;
            c_mul(_prod104r, _prod104i, k, 0, &_prod105r, &_prod105i);
            double _abs106r = 0, _abs106i = 0;
            _abs106r = c_abs(x1r, x1i); _abs106i = 0;
            double _abs107r = 0, _abs107i = 0;
            _abs107r = c_abs(x2r, x2i); _abs107i = 0;
            double _sum108r = 0, _sum108i = 0;
            _sum108r = _abs106r + _abs107r; _sum108i = _abs106i + _abs107i;
            double _sum109r = 0, _sum109i = 0;
            _sum109r = _sum108r + k; _sum109i = _sum108i + 0;
            double _c110r = 0, _c110i = 0;
            _c110r = 0.0; _c110i = 1.0;
            double _mul111r = 0, _mul111i = 0;
            c_mul(_sum109r, _sum109i, _c110r, _c110i, &_mul111r, &_mul111i);
            double _add112r = 0, _add112i = 0;
            _add112r = _prod105r + _mul111r; _add112i = _prod105i + _mul111i;
            { int _idx = (k - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add112r; cIm[_idx] = _add112i; } }
        }
    }
    double _c113r = 0, _c113i = 0;
    _c113r = 10.0; _c113i = 0;
    double _mul114r = 0, _mul114i = 0;
    c_mul(_c113r, _c113i, x1r, x1i, &_mul114r, &_mul114i);
    double _c115r = 0, _c115i = 0;
    _c115r = 0.0; _c115i = 5.0;
    double _c116r = 0, _c116i = 0;
    _c116r = 2.0; _c116i = 0;
    double _pow117r = 0, _pow117i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow117r, &_pow117i);
    double _mul118r = 0, _mul118i = 0;
    c_mul(_c115r, _c115i, _pow117r, _pow117i, &_mul118r, &_mul118i);
    double _sub119r = 0, _sub119i = 0;
    _sub119r = _mul114r - _mul118r; _sub119i = _mul114i - _mul118i;
    { int _idx = 4; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub119r; cIm[_idx] = _sub119i; } }
    double _c120r = 0, _c120i = 0;
    _c120r = 0.0; _c120i = 15.0;
    double _c121r = 0, _c121i = 0;
    _c121r = 3.0; _c121i = 0;
    double _pow122r = 0, _pow122i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow122r, &_pow122i);
    c_mul(_pow122r, _pow122i, x1r, x1i, &_pow122r, &_pow122i);
    double _mul123r = 0, _mul123i = 0;
    c_mul(_c120r, _c120i, _pow122r, _pow122i, &_mul123r, &_mul123i);
    double _c124r = 0, _c124i = 0;
    _c124r = 8.0; _c124i = 0;
    double _mul125r = 0, _mul125i = 0;
    c_mul(_c124r, _c124i, x2r, x2i, &_mul125r, &_mul125i);
    double _add126r = 0, _add126i = 0;
    _add126r = _mul123r + _mul125r; _add126i = _mul123i + _mul125i;
    { int _idx = 9; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add126r; cIm[_idx] = _add126i; } }
    double _c127r = 0, _c127i = 0;
    _c127r = 20.0; _c127i = 0;
    double _c128r = 0, _c128i = 0;
    _c128r = 2.0; _c128i = 0;
    double _pow129r = 0, _pow129i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow129r, &_pow129i);
    double _mul130r = 0, _mul130i = 0;
    c_mul(_c127r, _c127i, _pow129r, _pow129i, &_mul130r, &_mul130i);
    double _c131r = 0, _c131i = 0;
    _c131r = 0.0; _c131i = 10.0;
    double _c132r = 0, _c132i = 0;
    _c132r = 3.0; _c132i = 0;
    double _pow133r = 0, _pow133i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow133r, &_pow133i);
    c_mul(_pow133r, _pow133i, x2r, x2i, &_pow133r, &_pow133i);
    double _mul134r = 0, _mul134i = 0;
    c_mul(_c131r, _c131i, _pow133r, _pow133i, &_mul134r, &_mul134i);
    double _sub135r = 0, _sub135i = 0;
    _sub135r = _mul130r - _mul134r; _sub135i = _mul130i - _mul134i;
    { int _idx = 14; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub135r; cIm[_idx] = _sub135i; } }
    double _c136r = 0, _c136i = 0;
    _c136r = 0.0; _c136i = 25.0;
    double _mul137r = 0, _mul137i = 0;
    c_mul(_c136r, _c136i, x1r, x1i, &_mul137r, &_mul137i);
    double _c138r = 0, _c138i = 0;
    _c138r = 12.0; _c138i = 0;
    double _c139r = 0, _c139i = 0;
    _c139r = 2.0; _c139i = 0;
    double _pow140r = 0, _pow140i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow140r, &_pow140i);
    double _mul141r = 0, _mul141i = 0;
    c_mul(_c138r, _c138i, _pow140r, _pow140i, &_mul141r, &_mul141i);
    double _sub142r = 0, _sub142i = 0;
    _sub142r = _mul137r - _mul141r; _sub142i = _mul137i - _mul141i;
    { int _idx = 19; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub142r; cIm[_idx] = _sub142i; } }
    double _c143r = 0, _c143i = 0;
    _c143r = 30.0; _c143i = 0;
    double _c144r = 0, _c144i = 0;
    _c144r = 4.0; _c144i = 0;
    double _pow145r = 0, _pow145i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow145r, &_pow145i);
    c_mul(_pow145r, _pow145i, _pow145r, _pow145i, &_pow145r, &_pow145i);
    double _mul146r = 0, _mul146i = 0;
    c_mul(_c143r, _c143i, _pow145r, _pow145i, &_mul146r, &_mul146i);
    double _c147r = 0, _c147i = 0;
    _c147r = 0.0; _c147i = 15.0;
    double _mul148r = 0, _mul148i = 0;
    c_mul(_c147r, _c147i, x2r, x2i, &_mul148r, &_mul148i);
    double _add149r = 0, _add149i = 0;
    _add149r = _mul146r + _mul148r; _add149i = _mul146i + _mul148i;
    { int _idx = 24; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add149r; cIm[_idx] = _add149i; } }
    double _c150r = 0, _c150i = 0;
    _c150r = 0.0; _c150i = 35.0;
    double _c151r = 0, _c151i = 0;
    _c151r = 2.0; _c151i = 0;
    double _pow152r = 0, _pow152i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow152r, &_pow152i);
    double _mul153r = 0, _mul153i = 0;
    c_mul(_c150r, _c150i, _pow152r, _pow152i, &_mul153r, &_mul153i);
    double _c154r = 0, _c154i = 0;
    _c154r = 18.0; _c154i = 0;
    double _c155r = 0, _c155i = 0;
    _c155r = 3.0; _c155i = 0;
    double _pow156r = 0, _pow156i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow156r, &_pow156i);
    c_mul(_pow156r, _pow156i, x2r, x2i, &_pow156r, &_pow156i);
    double _mul157r = 0, _mul157i = 0;
    c_mul(_c154r, _c154i, _pow156r, _pow156i, &_mul157r, &_mul157i);
    double _sub158r = 0, _sub158i = 0;
    _sub158r = _mul153r - _mul157r; _sub158i = _mul153i - _mul157i;
    { int _idx = 34; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub158r; cIm[_idx] = _sub158i; } }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_234_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 36; j++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 2.0; _c1i = 0;
        double _add2r = 0, _add2i = 0;
        _add2r = j + _c1r; _add2i = 0 + _c1i;
        double k_r = _add2r, k_i = _add2i;
        double _sqrt3r = 0, _sqrt3i = 0;
        c_powr(j, 0, 0.5, &_sqrt3r, &_sqrt3i);
        double r_r = _sqrt3r, r_i = _sqrt3i;
        double _abs4r = 0, _abs4i = 0;
        _abs4r = c_abs(x1r, x1i); _abs4i = 0;
        double _pow5r = 0, _pow5i = 0;
        c_powc(_abs4r, _abs4i, r_r, r_i, &_pow5r, &_pow5i);
        double _abs6r = 0, _abs6i = 0;
        _abs6r = c_abs(x2r, x2i); _abs6i = 0;
        double _c7r = 0, _c7i = 0;
        _c7r = 5.0; _c7i = 0;
        double _mod8r = 0, _mod8i = 0;
        _mod8r = fmod(k_r, _c7r); _mod8i = 0;
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = _mod8r + _c9r; _add10i = _mod8i + _c9i;
        double _pow11r = 0, _pow11i = 0;
        c_powc(_abs6r, _abs6i, _add10r, _add10i, &_pow11r, &_pow11i);
        double _add12r = 0, _add12i = 0;
        _add12r = _pow5r + _pow11r; _add12i = _pow5i + _pow11i;
        double _sin13r = 0, _sin13i = 0;
        c_sin(j, 0, &_sin13r, &_sin13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_add12r, _add12i, _sin13r, _sin13i, &_mul14r, &_mul14i);
        double _abs15r = 0, _abs15i = 0;
        _abs15r = c_abs(x1r, x1i); _abs15i = 0;
        double _c16r = 0, _c16i = 0;
        _c16r = 1.0; _c16i = 0;
        double _add17r = 0, _add17i = 0;
        _add17r = _abs15r + _c16r; _add17i = _abs15i + _c16i;
        double _log18r = 0, _log18i = 0;
        c_log(_add17r, _add17i, &_log18r, &_log18i);
        double _cos19r = 0, _cos19i = 0;
        c_cos(r_r, r_i, &_cos19r, &_cos19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_log18r, _log18i, _cos19r, _cos19i, &_mul20r, &_mul20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _mul14r + _mul20r; _add21i = _mul14i + _mul20i;
        double mag_r = _add21r, mag_i = _add21i;
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x1r, x1i); _ang22i = 0;
        double _c23r = 0, _c23i = 0;
        _c23r = 2.0; _c23i = 0;
        double _div24r = 0, _div24i = 0;
        c_div(j, 0, _c23r, _c23i, &_div24r, &_div24i);
        double _cos25r = 0, _cos25i = 0;
        c_cos(_div24r, _div24i, &_cos25r, &_cos25i);
        double _mul26r = 0, _mul26i = 0;
        c_mul(_ang22r, _ang22i, _cos25r, _cos25i, &_mul26r, &_mul26i);
        double _ang27r = 0, _ang27i = 0;
        _ang27r = c_arg(x2r, x2i); _ang27i = 0;
        double _sin28r = 0, _sin28i = 0;
        c_sin(r_r, r_i, &_sin28r, &_sin28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_ang27r, _ang27i, _sin28r, _sin28i, &_mul29r, &_mul29i);
        double _sub30r = 0, _sub30i = 0;
        _sub30r = _mul26r - _mul29r; _sub30i = _mul26i - _mul29i;
        double ang_r = _sub30r, ang_i = _sub30i;
        double _cos31r = 0, _cos31i = 0;
        c_cos(ang_r, ang_i, &_cos31r, &_cos31i);
        double _sin32r = 0, _sin32i = 0;
        c_sin(ang_r, ang_i, &_sin32r, &_sin32i);
        double _c33r = 0, _c33i = 0;
        _c33r = 0.0; _c33i = 1.0;
        double _mul34r = 0, _mul34i = 0;
        c_mul(_sin32r, _sin32i, _c33r, _c33i, &_mul34r, &_mul34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _cos31r + _mul34r; _add35i = _cos31i + _mul34i;
        double _mul36r = 0, _mul36i = 0;
        c_mul(mag_r, mag_i, _add35r, _add35i, &_mul36r, &_mul36i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul36r; cIm[_idx] = _mul36i; } }
        double _conj37r = 0, _conj37i = 0;
        _conj37r = x1r; _conj37i = -(x1i);
        double _pow38r = 0, _pow38i = 0;
        c_powc(x2r, x2i, k_r, k_i, &_pow38r, &_pow38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(_conj37r, _conj37i, _pow38r, _pow38i, &_mul39r, &_mul39i);
        double _conj40r = 0, _conj40i = 0;
        _conj40r = x2r; _conj40i = -(x2i);
        double _c41r = 0, _c41i = 0;
        _c41r = 3.0; _c41i = 0;
        double _mod42r = 0, _mod42i = 0;
        _mod42r = fmod(k_r, _c41r); _mod42i = 0;
        double _pow43r = 0, _pow43i = 0;
        c_powc(x1r, x1i, _mod42r, _mod42i, &_pow43r, &_pow43i);
        double _mul44r = 0, _mul44i = 0;
        c_mul(_conj40r, _conj40i, _pow43r, _pow43i, &_mul44r, &_mul44i);
        double _sub45r = 0, _sub45i = 0;
        _sub45r = _mul39r - _mul44r; _sub45i = _mul39i - _mul44i;
        cRe[(j - 1)] += _sub45r; cIm[(j - 1)] += _sub45i;
    }
    double _re46r = 0, _re46i = 0;
    _re46r = x1r; _re46i = 0;
    double _im47r = 0, _im47i = 0;
    _im47r = x2i; _im47i = 0;
    double _c48r = 0, _c48i = 0;
    _c48r = 0.0; _c48i = 1.0;
    double _mul49r = 0, _mul49i = 0;
    c_mul(_im47r, _im47i, _c48r, _c48i, &_mul49r, &_mul49i);
    double _add50r = 0, _add50i = 0;
    _add50r = _re46r + _mul49r; _add50i = _re46i + _mul49i;
    { int _idx = 4; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add50r; cIm[_idx] = _add50i; } }
    double _abs51r = 0, _abs51i = 0;
    _abs51r = c_abs(x1r, x1i); _abs51i = 0;
    double _c52r = 0, _c52i = 0;
    _c52r = 1.0; _c52i = 0;
    double _add53r = 0, _add53i = 0;
    _add53r = _abs51r + _c52r; _add53i = _abs51i + _c52i;
    double _log54r = 0, _log54i = 0;
    c_log(_add53r, _add53i, &_log54r, &_log54i);
    double _abs55r = 0, _abs55i = 0;
    _abs55r = c_abs(x2r, x2i); _abs55i = 0;
    double _c56r = 0, _c56i = 0;
    _c56r = 1.0; _c56i = 0;
    double _add57r = 0, _add57i = 0;
    _add57r = _abs55r + _c56r; _add57i = _abs55i + _c56i;
    double _log58r = 0, _log58i = 0;
    c_log(_add57r, _add57i, &_log58r, &_log58i);
    double _mul59r = 0, _mul59i = 0;
    c_mul(_log54r, _log54i, _log58r, _log58i, &_mul59r, &_mul59i);
    double _mul60r = 0, _mul60i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul60r, &_mul60i);
    double _sin61r = 0, _sin61i = 0;
    c_sin(_mul60r, _mul60i, &_sin61r, &_sin61i);
    double _add62r = 0, _add62i = 0;
    _add62r = _mul59r + _sin61r; _add62i = _mul59i + _sin61i;
    { int _idx = 11; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add62r; cIm[_idx] = _add62i; } }
    double _abs63r = 0, _abs63i = 0;
    _abs63r = c_abs(x1r, x1i); _abs63i = 0;
    double _abs64r = 0, _abs64i = 0;
    _abs64r = c_abs(x2r, x2i); _abs64i = 0;
    double _sub65r = 0, _sub65i = 0;
    _sub65r = _abs63r - _abs64r; _sub65i = _abs63i - _abs64i;
    double _ang66r = 0, _ang66i = 0;
    _ang66r = c_arg(x1r, x1i); _ang66i = 0;
    double _ang67r = 0, _ang67i = 0;
    _ang67r = c_arg(x2r, x2i); _ang67i = 0;
    double _sub68r = 0, _sub68i = 0;
    _sub68r = _ang66r - _ang67r; _sub68i = _ang66i - _ang67i;
    double _cos69r = 0, _cos69i = 0;
    c_cos(_sub68r, _sub68i, &_cos69r, &_cos69i);
    double _mul70r = 0, _mul70i = 0;
    c_mul(_sub65r, _sub65i, _cos69r, _cos69i, &_mul70r, &_mul70i);
    double _c71r = 0, _c71i = 0;
    _c71r = 0.0; _c71i = 2.0;
    double _ang72r = 0, _ang72i = 0;
    _ang72r = c_arg(x1r, x1i); _ang72i = 0;
    double _ang73r = 0, _ang73i = 0;
    _ang73r = c_arg(x2r, x2i); _ang73i = 0;
    double _add74r = 0, _add74i = 0;
    _add74r = _ang72r + _ang73r; _add74i = _ang72i + _ang73i;
    double _sin75r = 0, _sin75i = 0;
    c_sin(_add74r, _add74i, &_sin75r, &_sin75i);
    double _mul76r = 0, _mul76i = 0;
    c_mul(_c71r, _c71i, _sin75r, _sin75i, &_mul76r, &_mul76i);
    double _add77r = 0, _add77i = 0;
    _add77r = _mul70r + _mul76r; _add77i = _mul70i + _mul76i;
    { int _idx = 19; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add77r; cIm[_idx] = _add77i; } }
    double _add78r = 0, _add78i = 0;
    _add78r = x1r + x2r; _add78i = x1i + x2i;
    double _conj79r = 0, _conj79i = 0;
    _conj79r = _add78r; _conj79i = -(_add78i);
    double _re80r = 0, _re80i = 0;
    _re80r = x1r; _re80i = 0;
    double _im81r = 0, _im81i = 0;
    _im81r = x2i; _im81i = 0;
    double _sub82r = 0, _sub82i = 0;
    _sub82r = _re80r - _im81r; _sub82i = _re80i - _im81i;
    double _mul83r = 0, _mul83i = 0;
    c_mul(_conj79r, _conj79i, _sub82r, _sub82i, &_mul83r, &_mul83i);
    double _c84r = 0, _c84i = 0;
    _c84r = 0.0; _c84i = 3.0;
    double _add85r = 0, _add85i = 0;
    _add85r = _mul83r + _c84r; _add85i = _mul83i + _c84i;
    { int _idx = 24; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add85r; cIm[_idx] = _add85i; } }
    double _abs86r = 0, _abs86i = 0;
    _abs86r = c_abs(x1r, x1i); _abs86i = 0;
    double _abs87r = 0, _abs87i = 0;
    _abs87r = c_abs(x2r, x2i); _abs87i = 0;
    double _sum88r = 0, _sum88i = 0;
    _sum88r = _abs86r + _abs87r; _sum88i = _abs86i + _abs87i;
    double _abs89r = 0, _abs89i = 0;
    _abs89r = c_abs(x1r, x1i); _abs89i = 0;
    double _abs90r = 0, _abs90i = 0;
    _abs90r = c_abs(x2r, x2i); _abs90i = 0;
    double _prod91r = 0, _prod91i = 0;
    c_mul(_abs89r, _abs89i, _abs90r, _abs90i, &_prod91r, &_prod91i);
    double _c92r = 0, _c92i = 0;
    _c92r = 0.0; _c92i = 1.0;
    double _mul93r = 0, _mul93i = 0;
    c_mul(_prod91r, _prod91i, _c92r, _c92i, &_mul93r, &_mul93i);
    double _add94r = 0, _add94i = 0;
    _add94r = _sum88r + _mul93r; _add94i = _sum88i + _mul93i;
    { int _idx = 29; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add94r; cIm[_idx] = _add94i; } }
    double _mul95r = 0, _mul95i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul95r, &_mul95i);
    double _ang96r = 0, _ang96i = 0;
    _ang96r = c_arg(_mul95r, _mul95i); _ang96i = 0;
    double _add97r = 0, _add97i = 0;
    _add97r = x1r + x2r; _add97i = x1i + x2i;
    double _abs98r = 0, _abs98i = 0;
    _abs98r = c_abs(_add97r, _add97i); _abs98i = 0;
    double _c99r = 0, _c99i = 0;
    _c99r = 0.0; _c99i = 1.0;
    double _mul100r = 0, _mul100i = 0;
    c_mul(_abs98r, _abs98i, _c99r, _c99i, &_mul100r, &_mul100i);
    double _add101r = 0, _add101i = 0;
    _add101r = _ang96r + _mul100r; _add101i = _ang96i + _mul100i;
    { int _idx = 34; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add101r; cIm[_idx] = _add101i; } }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_235_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 3.0; _c1i = 0;
    { int _idx = 2; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _c1r; cIm[_idx] = _c1i; } }
    double _c2r = 0, _c2i = 0;
    _c2r = 5.0; _c2i = 0;
    double _neg3r = 0, _neg3i = 0;
    _neg3r = -(_c2r); _neg3i = -(_c2i);
    { int _idx = 6; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _neg3r; cIm[_idx] = _neg3i; } }
    double _c4r = 0, _c4i = 0;
    _c4r = 7.0; _c4i = 0;
    { int _idx = 11; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _c4r; cIm[_idx] = _c4i; } }
    double _c5r = 0, _c5i = 0;
    _c5r = 11.0; _c5i = 0;
    double _neg6r = 0, _neg6i = 0;
    _neg6r = -(_c5r); _neg6i = -(_c5i);
    { int _idx = 18; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _neg6r; cIm[_idx] = _neg6i; } }
    double _c7r = 0, _c7i = 0;
    _c7r = 13.0; _c7i = 0;
    { int _idx = 24; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _c7r; cIm[_idx] = _c7i; } }
    double _c8r = 0, _c8i = 0;
    _c8r = 17.0; _c8i = 0;
    double _neg9r = 0, _neg9i = 0;
    _neg9r = -(_c8r); _neg9i = -(_c8i);
    { int _idx = 29; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _neg9r; cIm[_idx] = _neg9i; } }
    for (int j = 1; j < 11; j++) {
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(x1r, x1i); _abs10i = 0;
        double _pow11r = 0, _pow11i = 0;
        c_powr(_abs10r, _abs10i, j, &_pow11r, &_pow11i);
        double _c12r = 0, _c12i = 0;
        _c12r = 1.0; _c12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = _pow11r + _c12r; _add13i = _pow11i + _c12i;
        double _log14r = 0, _log14i = 0;
        c_log(_add13r, _add13i, &_log14r, &_log14i);
        double _abs15r = 0, _abs15i = 0;
        _abs15r = c_abs(x2r, x2i); _abs15i = 0;
        double _c16r = 0, _c16i = 0;
        _c16r = 3.0; _c16i = 0;
        double _mod17r = 0, _mod17i = 0;
        _mod17r = fmod(j, _c16r); _mod17i = 0;
        double _c18r = 0, _c18i = 0;
        _c18r = 1.0; _c18i = 0;
        double _add19r = 0, _add19i = 0;
        _add19r = _mod17r + _c18r; _add19i = _mod17i + _c18i;
        double _pow20r = 0, _pow20i = 0;
        c_powc(_abs15r, _abs15i, _add19r, _add19i, &_pow20r, &_pow20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _log14r + _pow20r; _add21i = _log14i + _pow20i;
        double magnitude_r = _add21r, magnitude_i = _add21i;
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x1r, x1i); _ang22i = 0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(_ang22r, _ang22i, j, 0, &_mul23r, &_mul23i);
        double _ang24r = 0, _ang24i = 0;
        _ang24r = c_arg(x2r, x2i); _ang24i = 0;
        double _c25r = 0, _c25i = 0;
        _c25r = 2.0; _c25i = 0;
        double _mod26r = 0, _mod26i = 0;
        _mod26r = fmod(j, _c25r); _mod26i = 0;
        double _mul27r = 0, _mul27i = 0;
        c_mul(_ang24r, _ang24i, _mod26r, _mod26i, &_mul27r, &_mul27i);
        double _sub28r = 0, _sub28i = 0;
        _sub28r = _mul23r - _mul27r; _sub28i = _mul23i - _mul27i;
        double angle_r = _sub28r, angle_i = _sub28i;
        double _cos29r = 0, _cos29i = 0;
        c_cos(angle_r, angle_i, &_cos29r, &_cos29i);
        double _c30r = 0, _c30i = 0;
        _c30r = 0.0; _c30i = 1.0;
        double _sin31r = 0, _sin31i = 0;
        c_sin(angle_r, angle_i, &_sin31r, &_sin31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_c30r, _c30i, _sin31r, _sin31i, &_mul32r, &_mul32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _cos29r + _mul32r; _add33i = _cos29i + _mul32i;
        double _mul34r = 0, _mul34i = 0;
        c_mul(magnitude_r, magnitude_i, _add33r, _add33i, &_mul34r, &_mul34i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul34r; cIm[_idx] = _mul34i; } }
    }
    for (int k = 11; k < 21; k++) {
        double _re35r = 0, _re35i = 0;
        _re35r = x1r; _re35i = 0;
        double _mul36r = 0, _mul36i = 0;
        c_mul(_re35r, _re35i, k, 0, &_mul36r, &_mul36i);
        double _sin37r = 0, _sin37i = 0;
        c_sin(_mul36r, _mul36i, &_sin37r, &_sin37i);
        double _im38r = 0, _im38i = 0;
        _im38r = x2i; _im38i = 0;
        double _mul39r = 0, _mul39i = 0;
        c_mul(_im38r, _im38i, k, 0, &_mul39r, &_mul39i);
        double _cos40r = 0, _cos40i = 0;
        c_cos(_mul39r, _mul39i, &_cos40r, &_cos40i);
        double _add41r = 0, _add41i = 0;
        _add41r = _sin37r + _cos40r; _add41i = _sin37i + _cos40i;
        magnitude_r = _add41r; magnitude_i = _add41i;
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
        double _mul47r = 0, _mul47i = 0;
        c_mul(_log46r, _log46i, k, 0, &_mul47r, &_mul47i);
        angle_r = _mul47r; angle_i = _mul47i;
        double _cos48r = 0, _cos48i = 0;
        c_cos(angle_r, angle_i, &_cos48r, &_cos48i);
        double _c49r = 0, _c49i = 0;
        _c49r = 0.0; _c49i = 1.0;
        double _sin50r = 0, _sin50i = 0;
        c_sin(angle_r, angle_i, &_sin50r, &_sin50i);
        double _mul51r = 0, _mul51i = 0;
        c_mul(_c49r, _c49i, _sin50r, _sin50i, &_mul51r, &_mul51i);
        double _add52r = 0, _add52i = 0;
        _add52r = _cos48r + _mul51r; _add52i = _cos48i + _mul51i;
        double _mul53r = 0, _mul53i = 0;
        c_mul(magnitude_r, magnitude_i, _add52r, _add52i, &_mul53r, &_mul53i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul53r; cIm[_idx] = _mul53i; } }
    }
    for (int r = 21; r < 35; r++) {
        double _mul54r = 0, _mul54i = 0;
        c_mul(x1r, x1i, r, 0, &_mul54r, &_mul54i);
        double _abs55r = 0, _abs55i = 0;
        _abs55r = c_abs(_mul54r, _mul54i); _abs55i = 0;
        double _c56r = 0, _c56i = 0;
        _c56r = 1.0; _c56i = 0;
        double _add57r = 0, _add57i = 0;
        _add57r = _abs55r + _c56r; _add57i = _abs55i + _c56i;
        double _log58r = 0, _log58i = 0;
        c_log(_add57r, _add57i, &_log58r, &_log58i);
        double _re59r = 0, _re59i = 0;
        _re59r = x2r; _re59i = 0;
        double _c60r = 0, _c60i = 0;
        _c60r = 2.0; _c60i = 0;
        double _pow61r = 0, _pow61i = 0;
        c_mul(_re59r, _re59i, _re59r, _re59i, &_pow61r, &_pow61i);
        double _add62r = 0, _add62i = 0;
        _add62r = _log58r + _pow61r; _add62i = _log58i + _pow61i;
        magnitude_r = _add62r; magnitude_i = _add62i;
        double _conj63r = 0, _conj63i = 0;
        _conj63r = x1r; _conj63i = -(x1i);
        double _conj64r = 0, _conj64i = 0;
        _conj64r = x2r; _conj64i = -(x2i);
        double _add65r = 0, _add65i = 0;
        _add65r = _conj63r + _conj64r; _add65i = _conj63i + _conj64i;
        double _ang66r = 0, _ang66i = 0;
        _ang66r = c_arg(_add65r, _add65i); _ang66i = 0;
        double _mul67r = 0, _mul67i = 0;
        c_mul(_ang66r, _ang66i, r, 0, &_mul67r, &_mul67i);
        angle_r = _mul67r; angle_i = _mul67i;
        double _cos68r = 0, _cos68i = 0;
        c_cos(angle_r, angle_i, &_cos68r, &_cos68i);
        double _c69r = 0, _c69i = 0;
        _c69r = 0.0; _c69i = 1.0;
        double _sin70r = 0, _sin70i = 0;
        c_sin(angle_r, angle_i, &_sin70r, &_sin70i);
        double _mul71r = 0, _mul71i = 0;
        c_mul(_c69r, _c69i, _sin70r, _sin70i, &_mul71r, &_mul71i);
        double _add72r = 0, _add72i = 0;
        _add72r = _cos68r + _mul71r; _add72i = _cos68i + _mul71i;
        double _mul73r = 0, _mul73i = 0;
        c_mul(magnitude_r, magnitude_i, _add72r, _add72i, &_mul73r, &_mul73i);
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul73r; cIm[_idx] = _mul73i; } }
    }
    double _mul74r = 0, _mul74i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul74r, &_mul74i);
    double _sin75r = 0, _sin75i = 0;
    c_sin(_mul74r, _mul74i, &_sin75r, &_sin75i);
    double _c76r = 0, _c76i = 0;
    _c76r = 1.0; _c76i = 0;
    double _add77r = 0, _add77i = 0;
    _add77r = x2r + _c76r; _add77i = x2i + _c76i;
    double _div78r = 0, _div78i = 0;
    c_div(x1r, x1i, _add77r, _add77i, &_div78r, &_div78i);
    double _cos79r = 0, _cos79i = 0;
    c_cos(_div78r, _div78i, &_cos79r, &_cos79i);
    double _add80r = 0, _add80i = 0;
    _add80r = _sin75r + _cos79r; _add80i = _sin75i + _cos79i;
    double _c81r = 0, _c81i = 0;
    _c81r = 0.0; _c81i = 1.0;
    double _add82r = 0, _add82i = 0;
    _add82r = x1r + x2r; _add82i = x1i + x2i;
    double _abs83r = 0, _abs83i = 0;
    _abs83r = c_abs(_add82r, _add82i); _abs83i = 0;
    double _c84r = 0, _c84i = 0;
    _c84r = 1.0; _c84i = 0;
    double _add85r = 0, _add85i = 0;
    _add85r = _abs83r + _c84r; _add85i = _abs83i + _c84i;
    double _log86r = 0, _log86i = 0;
    c_log(_add85r, _add85i, &_log86r, &_log86i);
    double _mul87r = 0, _mul87i = 0;
    c_mul(_c81r, _c81i, _log86r, _log86i, &_mul87r, &_mul87i);
    double _add88r = 0, _add88i = 0;
    _add88r = _add80r + _mul87r; _add88i = _add80i + _mul87i;
    { int _idx = 34; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add88r; cIm[_idx] = _add88i; } }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_236: auto-stubbed (unhandled constructs in source) */
static void poly_236_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_237_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 36; j++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 4.0; _c1i = 0;
        double _mod2r = 0, _mod2i = 0;
        _mod2r = fmod(j, _c1r); _mod2i = 0;
        double _c3r = 0, _c3i = 0;
        _c3r = 1.0; _c3i = 0;
        if (_mod2r == _c3r) {
            double _pow4r = 0, _pow4i = 0;
            c_powr(x1r, x1i, j, &_pow4r, &_pow4i);
            double _ang5r = 0, _ang5i = 0;
            _ang5r = c_arg(x2r, x2i); _ang5i = 0;
            double _mul6r = 0, _mul6i = 0;
            c_mul(j, 0, _ang5r, _ang5i, &_mul6r, &_mul6i);
            double _sin7r = 0, _sin7i = 0;
            c_sin(_mul6r, _mul6i, &_sin7r, &_sin7i);
            double _mul8r = 0, _mul8i = 0;
            c_mul(_pow4r, _pow4i, _sin7r, _sin7i, &_mul8r, &_mul8i);
            double _conj9r = 0, _conj9i = 0;
            _conj9r = x2r; _conj9i = -(x2i);
            double _c10r = 0, _c10i = 0;
            _c10r = 2.0; _c10i = 0;
            double _pow11r = 0, _pow11i = 0;
            c_mul(_conj9r, _conj9i, _conj9r, _conj9i, &_pow11r, &_pow11i);
            double _add12r = 0, _add12i = 0;
            _add12r = _mul8r + _pow11r; _add12i = _mul8i + _pow11i;
            { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add12r; cIm[_idx] = _add12i; } }
        } else {
            double _c13r = 0, _c13i = 0;
            _c13r = 4.0; _c13i = 0;
            double _mod14r = 0, _mod14i = 0;
            _mod14r = fmod(j, _c13r); _mod14i = 0;
            double _c15r = 0, _c15i = 0;
            _c15r = 2.0; _c15i = 0;
            if (_mod14r == _c15r) {
                double _pow16r = 0, _pow16i = 0;
                c_powr(x2r, x2i, j, &_pow16r, &_pow16i);
                double _ang17r = 0, _ang17i = 0;
                _ang17r = c_arg(x1r, x1i); _ang17i = 0;
                double _mul18r = 0, _mul18i = 0;
                c_mul(j, 0, _ang17r, _ang17i, &_mul18r, &_mul18i);
                double _cos19r = 0, _cos19i = 0;
                c_cos(_mul18r, _mul18i, &_cos19r, &_cos19i);
                double _mul20r = 0, _mul20i = 0;
                c_mul(_pow16r, _pow16i, _cos19r, _cos19i, &_mul20r, &_mul20i);
                double _conj21r = 0, _conj21i = 0;
                _conj21r = x1r; _conj21i = -(x1i);
                double _c22r = 0, _c22i = 0;
                _c22r = 2.0; _c22i = 0;
                double _pow23r = 0, _pow23i = 0;
                c_mul(_conj21r, _conj21i, _conj21r, _conj21i, &_pow23r, &_pow23i);
                double _add24r = 0, _add24i = 0;
                _add24r = _mul20r + _pow23r; _add24i = _mul20i + _pow23i;
                { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add24r; cIm[_idx] = _add24i; } }
            } else {
                double _c25r = 0, _c25i = 0;
                _c25r = 4.0; _c25i = 0;
                double _mod26r = 0, _mod26i = 0;
                _mod26r = fmod(j, _c25r); _mod26i = 0;
                double _c27r = 0, _c27i = 0;
                _c27r = 3.0; _c27i = 0;
                if (_mod26r == _c27r) {
                    double _re28r = 0, _re28i = 0;
                    _re28r = x1r; _re28i = 0;
                    double _im29r = 0, _im29i = 0;
                    _im29r = x2i; _im29i = 0;
                    double _mul30r = 0, _mul30i = 0;
                    c_mul(_re28r, _re28i, _im29r, _im29i, &_mul30r, &_mul30i);
                    double _abs31r = 0, _abs31i = 0;
                    _abs31r = c_abs(x1r, x1i); _abs31i = 0;
                    double _c32r = 0, _c32i = 0;
                    _c32r = 1.0; _c32i = 0;
                    double _add33r = 0, _add33i = 0;
                    _add33r = _abs31r + _c32r; _add33i = _abs31i + _c32i;
                    double _log34r = 0, _log34i = 0;
                    c_log(_add33r, _add33i, &_log34r, &_log34i);
                    double _mul35r = 0, _mul35i = 0;
                    c_mul(_mul30r, _mul30i, _log34r, _log34i, &_mul35r, &_mul35i);
                    double _re36r = 0, _re36i = 0;
                    _re36r = x2r; _re36i = 0;
                    double _pow37r = 0, _pow37i = 0;
                    c_powr(_re36r, _re36i, j, &_pow37r, &_pow37i);
                    double _add38r = 0, _add38i = 0;
                    _add38r = _mul35r + _pow37r; _add38i = _mul35i + _pow37i;
                    { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add38r; cIm[_idx] = _add38i; } }
                } else {
                    double _im39r = 0, _im39i = 0;
                    _im39r = x1i; _im39i = 0;
                    double _re40r = 0, _re40i = 0;
                    _re40r = x2r; _re40i = 0;
                    double _mul41r = 0, _mul41i = 0;
                    c_mul(_im39r, _im39i, _re40r, _re40i, &_mul41r, &_mul41i);
                    double _abs42r = 0, _abs42i = 0;
                    _abs42r = c_abs(x2r, x2i); _abs42i = 0;
                    double _c43r = 0, _c43i = 0;
                    _c43r = 1.0; _c43i = 0;
                    double _add44r = 0, _add44i = 0;
                    _add44r = _abs42r + _c43r; _add44i = _abs42i + _c43i;
                    double _log45r = 0, _log45i = 0;
                    c_log(_add44r, _add44i, &_log45r, &_log45i);
                    double _mul46r = 0, _mul46i = 0;
                    c_mul(_mul41r, _mul41i, _log45r, _log45i, &_mul46r, &_mul46i);
                    double _im47r = 0, _im47i = 0;
                    _im47r = x2i; _im47i = 0;
                    double _pow48r = 0, _pow48i = 0;
                    c_powr(_im47r, _im47i, j, &_pow48r, &_pow48i);
                    double _add49r = 0, _add49i = 0;
                    _add49r = _mul46r + _pow48r; _add49i = _mul46i + _pow48i;
                    { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add49r; cIm[_idx] = _add49i; } }
                }
            }
        }
        double _conj50r = 0, _conj50i = 0;
        _conj50r = x1r; _conj50i = -(x1i);
        double _conj51r = 0, _conj51i = 0;
        _conj51r = x2r; _conj51i = -(x2i);
        double _mul52r = 0, _mul52i = 0;
        c_mul(_conj50r, _conj50i, _conj51r, _conj51i, &_mul52r, &_mul52i);
        double _c53r = 0, _c53i = 0;
        _c53r = 1.0; _c53i = 0;
        double _add54r = 0, _add54i = 0;
        _add54r = j + _c53r; _add54i = 0 + _c53i;
        double _div55r = 0, _div55i = 0;
        c_div(_mul52r, _mul52i, _add54r, _add54i, &_div55r, &_div55i);
        cRe[(j - 1)] += _div55r; cIm[(j - 1)] += _div55i;
    }
    for (int k = 1; k < 36; k++) {
        double _c56r = 0, _c56i = 0;
        _c56r = 5.0; _c56i = 0;
        double _mod57r = 0, _mod57i = 0;
        _mod57r = fmod(k, _c56r); _mod57i = 0;
        double _c58r = 0, _c58i = 0;
        _c58r = 0.0; _c58i = 0;
        if (_mod57r == _c58r) {
            double _cf59r = 0, _cf59i = 0;
            { int _idx = (k - 1); if (_idx >= 0 && _idx < 35) { _cf59r = cRe[_idx]; _cf59i = cIm[_idx]; } }
            double _c60r = 0, _c60i = 0;
            _c60r = 1.0; _c60i = 0;
            double _c61r = 0, _c61i = 0;
            _c61r = 0.05; _c61i = 0;
            double _mul62r = 0, _mul62i = 0;
            c_mul(_c61r, _c61i, k, 0, &_mul62r, &_mul62i);
            double _add63r = 0, _add63i = 0;
            _add63r = _c60r + _mul62r; _add63i = _c60i + _mul62i;
            double _mul64r = 0, _mul64i = 0;
            c_mul(_cf59r, _cf59i, _add63r, _add63i, &_mul64r, &_mul64i);
            double _cf65r = 0, _cf65i = 0;
            { int _idx = (k - 1); if (_idx >= 0 && _idx < 35) { _cf65r = cRe[_idx]; _cf65i = cIm[_idx]; } }
            double _ang66r = 0, _ang66i = 0;
            _ang66r = c_arg(_cf65r, _cf65i); _ang66i = 0;
            double _mul67r = 0, _mul67i = 0;
            c_mul(k, 0, _ang66r, _ang66i, &_mul67r, &_mul67i);
            double _sin68r = 0, _sin68i = 0;
            c_sin(_mul67r, _mul67i, &_sin68r, &_sin68i);
            double _add69r = 0, _add69i = 0;
            _add69r = _mul64r + _sin68r; _add69i = _mul64i + _sin68i;
            { int _idx = (k - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add69r; cIm[_idx] = _add69i; } }
        } else {
            double _cf70r = 0, _cf70i = 0;
            { int _idx = (k - 1); if (_idx >= 0 && _idx < 35) { _cf70r = cRe[_idx]; _cf70i = cIm[_idx]; } }
            double _c71r = 0, _c71i = 0;
            _c71r = 1.0; _c71i = 0;
            double _c72r = 0, _c72i = 0;
            _c72r = 0.02; _c72i = 0;
            double _mul73r = 0, _mul73i = 0;
            c_mul(_c72r, _c72i, k, 0, &_mul73r, &_mul73i);
            double _add74r = 0, _add74i = 0;
            _add74r = _c71r + _mul73r; _add74i = _c71i + _mul73i;
            double _div75r = 0, _div75i = 0;
            c_div(_cf70r, _cf70i, _add74r, _add74i, &_div75r, &_div75i);
            double _cf76r = 0, _cf76i = 0;
            { int _idx = (k - 1); if (_idx >= 0 && _idx < 35) { _cf76r = cRe[_idx]; _cf76i = cIm[_idx]; } }
            double _ang77r = 0, _ang77i = 0;
            _ang77r = c_arg(_cf76r, _cf76i); _ang77i = 0;
            double _mul78r = 0, _mul78i = 0;
            c_mul(k, 0, _ang77r, _ang77i, &_mul78r, &_mul78i);
            double _cos79r = 0, _cos79i = 0;
            c_cos(_mul78r, _mul78i, &_cos79r, &_cos79i);
            double _add80r = 0, _add80i = 0;
            _add80r = _div75r + _cos79r; _add80i = _div75i + _cos79i;
            { int _idx = (k - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add80r; cIm[_idx] = _add80i; } }
        }
        double _cf81r = 0, _cf81i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 35) { _cf81r = cRe[_idx]; _cf81i = cIm[_idx]; } }
        double _abs82r = 0, _abs82i = 0;
        _abs82r = c_abs(_cf81r, _cf81i); _abs82i = 0;
        double _c83r = 0, _c83i = 0;
        _c83r = 1.0; _c83i = 0;
        double _add84r = 0, _add84i = 0;
        _add84r = _abs82r + _c83r; _add84i = _abs82i + _c83i;
        double _log85r = 0, _log85i = 0;
        c_log(_add84r, _add84i, &_log85r, &_log85i);
        double _cf86r = 0, _cf86i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 35) { _cf86r = cRe[_idx]; _cf86i = cIm[_idx]; } }
        double _re87r = 0, _re87i = 0;
        _re87r = _cf86r; _re87i = 0;
        double _mul88r = 0, _mul88i = 0;
        c_mul(_log85r, _log85i, _re87r, _re87i, &_mul88r, &_mul88i);
        cRe[(k - 1)] += _mul88r; cIm[(k - 1)] += _mul88i;
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_238_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 36; j++) {
        double _ang1r = 0, _ang1i = 0;
        _ang1r = c_arg(x1r, x1i); _ang1i = 0;
        double _c2r = 0, _c2i = 0;
        _c2r = 2.0; _c2i = 0;
        double _pow3r = 0, _pow3i = 0;
        c_mul(j, 0, j, 0, &_pow3r, &_pow3i);
        double _mul4r = 0, _mul4i = 0;
        c_mul(_ang1r, _ang1i, _pow3r, _pow3i, &_mul4r, &_mul4i);
        double _ang5r = 0, _ang5i = 0;
        _ang5r = c_arg(x2r, x2i); _ang5i = 0;
        double _sqrt6r = 0, _sqrt6i = 0;
        c_powr(j, 0, 0.5, &_sqrt6r, &_sqrt6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(_ang5r, _ang5i, _sqrt6r, _sqrt6i, &_mul7r, &_mul7i);
        double _sub8r = 0, _sub8i = 0;
        _sub8r = _mul4r - _mul7r; _sub8i = _mul4i - _mul7i;
        double angle_r = _sub8r, angle_i = _sub8i;
        double _abs9r = 0, _abs9i = 0;
        _abs9r = c_abs(x1r, x1i); _abs9i = 0;
        double _pow10r = 0, _pow10i = 0;
        c_powr(_abs9r, _abs9i, j, &_pow10r, &_pow10i);
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(x2r, x2i); _abs11i = 0;
        double _c12r = 0, _c12i = 0;
        _c12r = 35.0; _c12i = 0;
        double _sub13r = 0, _sub13i = 0;
        _sub13r = _c12r - j; _sub13i = _c12i - 0;
        double _pow14r = 0, _pow14i = 0;
        c_powc(_abs11r, _abs11i, _sub13r, _sub13i, &_pow14r, &_pow14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _pow10r + _pow14r; _add15i = _pow10i + _pow14i;
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
        double magnitude_r = _add21r, magnitude_i = _add21i;
        double _cos22r = 0, _cos22i = 0;
        c_cos(angle_r, angle_i, &_cos22r, &_cos22i);
        double _sin23r = 0, _sin23i = 0;
        c_sin(angle_r, angle_i, &_sin23r, &_sin23i);
        double _c24r = 0, _c24i = 0;
        _c24r = 0.0; _c24i = 1.0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(_sin23r, _sin23i, _c24r, _c24i, &_mul25r, &_mul25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _cos22r + _mul25r; _add26i = _cos22i + _mul25i;
        double _mul27r = 0, _mul27i = 0;
        c_mul(magnitude_r, magnitude_i, _add26r, _add26i, &_mul27r, &_mul27i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul27r; cIm[_idx] = _mul27i; } }
    }
    for (int k = 1; k < 36; k++) {
        double _sqrt28r = 0, _sqrt28i = 0;
        c_powr(k, 0, 0.5, &_sqrt28r, &_sqrt28i);
        double r_r = _sqrt28r, r_i = _sqrt28i;
        double _cf29r = 0, _cf29i = 0;
        { int _idx = (36 - k); if (_idx >= 0 && _idx < 35) { _cf29r = cRe[_idx]; _cf29i = cIm[_idx]; } }
        double _conj30r = 0, _conj30i = 0;
        _conj30r = _cf29r; _conj30i = -(_cf29i);
        double _c31r = 0, _c31i = 0;
        _c31r = 5.0; _c31i = 0;
        double _mod32r = 0, _mod32i = 0;
        _mod32r = fmod(k, _c31r); _mod32i = 0;
        double _pow33r = 0, _pow33i = 0;
        c_powc(x1r, x1i, _mod32r, _mod32i, &_pow33r, &_pow33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(_conj30r, _conj30i, _pow33r, _pow33i, &_mul34r, &_mul34i);
        double _cf35r = 0, _cf35i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 35) { _cf35r = cRe[_idx]; _cf35i = cIm[_idx]; } }
        double _conj36r = 0, _conj36i = 0;
        _conj36r = _cf35r; _conj36i = -(_cf35i);
        double _c37r = 0, _c37i = 0;
        _c37r = 35.0; _c37i = 0;
        double _c38r = 0, _c38i = 0;
        _c38r = 3.0; _c38i = 0;
        double _mod39r = 0, _mod39i = 0;
        _mod39r = fmod(k, _c38r); _mod39i = 0;
        double _sub40r = 0, _sub40i = 0;
        _sub40r = _c37r - _mod39r; _sub40i = _c37i - _mod39i;
        double _pow41r = 0, _pow41i = 0;
        c_powc(x2r, x2i, _sub40r, _sub40i, &_pow41r, &_pow41i);
        double _mul42r = 0, _mul42i = 0;
        c_mul(_conj36r, _conj36i, _pow41r, _pow41i, &_mul42r, &_mul42i);
        double _sub43r = 0, _sub43i = 0;
        _sub43r = _mul34r - _mul42r; _sub43i = _mul34i - _mul42i;
        cRe[(k - 1)] += _sub43r; cIm[(k - 1)] += _sub43i;
    }
    double _c44r = 0, _c44i = 0;
    _c44r = 2.0; _c44i = 0;
    double _c45r = 0, _c45i = 0;
    _c45r = 3.0; _c45i = 0;
    double _pow46r = 0, _pow46i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow46r, &_pow46i);
    c_mul(_pow46r, _pow46i, x1r, x1i, &_pow46r, &_pow46i);
    double _mul47r = 0, _mul47i = 0;
    c_mul(_c44r, _c44i, _pow46r, _pow46i, &_mul47r, &_mul47i);
    double _c48r = 0, _c48i = 0;
    _c48r = 3.0; _c48i = 0;
    double _c49r = 0, _c49i = 0;
    _c49r = 2.0; _c49i = 0;
    double _pow50r = 0, _pow50i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow50r, &_pow50i);
    double _mul51r = 0, _mul51i = 0;
    c_mul(_c48r, _c48i, _pow50r, _pow50i, &_mul51r, &_mul51i);
    double _sub52r = 0, _sub52i = 0;
    _sub52r = _mul47r - _mul51r; _sub52i = _mul47i - _mul51i;
    double _mul53r = 0, _mul53i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul53r, &_mul53i);
    double _sin54r = 0, _sin54i = 0;
    c_sin(_mul53r, _mul53i, &_sin54r, &_sin54i);
    double _c55r = 0, _c55i = 0;
    _c55r = 0.0; _c55i = 1.0;
    double _mul56r = 0, _mul56i = 0;
    c_mul(_sin54r, _sin54i, _c55r, _c55i, &_mul56r, &_mul56i);
    double _add57r = 0, _add57i = 0;
    _add57r = _sub52r + _mul56r; _add57i = _sub52i + _mul56i;
    cRe[4] += _add57r; cIm[4] += _add57i;
    double _cf58r = 0, _cf58i = 0;
    { int _idx = 9; if (_idx >= 0 && _idx < 35) { _cf58r = cRe[_idx]; _cf58i = cIm[_idx]; } }
    double _conj59r = 0, _conj59i = 0;
    _conj59r = _cf58r; _conj59i = -(_cf58i);
    double _mul60r = 0, _mul60i = 0;
    c_mul(_conj59r, _conj59i, x1r, x1i, &_mul60r, &_mul60i);
    double _cf61r = 0, _cf61i = 0;
    { int _idx = 9; if (_idx >= 0 && _idx < 35) { _cf61r = cRe[_idx]; _cf61i = cIm[_idx]; } }
    double _im62r = 0, _im62i = 0;
    _im62r = _cf61i; _im62i = 0;
    double _mul63r = 0, _mul63i = 0;
    c_mul(_im62r, _im62i, x2r, x2i, &_mul63r, &_mul63i);
    double _sub64r = 0, _sub64i = 0;
    _sub64r = _mul60r - _mul63r; _sub64i = _mul60i - _mul63i;
    double _add65r = 0, _add65i = 0;
    _add65r = x1r + x2r; _add65i = x1i + x2i;
    double _abs66r = 0, _abs66i = 0;
    _abs66r = c_abs(_add65r, _add65i); _abs66i = 0;
    double _c67r = 0, _c67i = 0;
    _c67r = 1.0; _c67i = 0;
    double _add68r = 0, _add68i = 0;
    _add68r = _abs66r + _c67r; _add68i = _abs66i + _c67i;
    double _log69r = 0, _log69i = 0;
    c_log(_add68r, _add68i, &_log69r, &_log69i);
    double _add70r = 0, _add70i = 0;
    _add70r = _sub64r + _log69r; _add70i = _sub64i + _log69i;
    { int _idx = 9; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add70r; cIm[_idx] = _add70i; } }
    double _cf71r = 0, _cf71i = 0;
    { int _idx = 14; if (_idx >= 0 && _idx < 35) { _cf71r = cRe[_idx]; _cf71i = cIm[_idx]; } }
    double _c72r = 0, _c72i = 0;
    _c72r = 2.0; _c72i = 0;
    double _pow73r = 0, _pow73i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow73r, &_pow73i);
    double _mul74r = 0, _mul74i = 0;
    c_mul(_cf71r, _cf71i, _pow73r, _pow73i, &_mul74r, &_mul74i);
    double _cf75r = 0, _cf75i = 0;
    { int _idx = 14; if (_idx >= 0 && _idx < 35) { _cf75r = cRe[_idx]; _cf75i = cIm[_idx]; } }
    double _abs76r = 0, _abs76i = 0;
    _abs76r = c_abs(x2r, x2i); _abs76i = 0;
    double _c77r = 0, _c77i = 0;
    _c77r = 1.0; _c77i = 0;
    double _add78r = 0, _add78i = 0;
    _add78r = _abs76r + _c77r; _add78i = _abs76i + _c77i;
    double _div79r = 0, _div79i = 0;
    c_div(_cf75r, _cf75i, _add78r, _add78i, &_div79r, &_div79i);
    double _sub80r = 0, _sub80i = 0;
    _sub80r = _mul74r - _div79r; _sub80i = _mul74i - _div79i;
    double _sub81r = 0, _sub81i = 0;
    _sub81r = x1r - x2r; _sub81i = x1i - x2i;
    double _cos82r = 0, _cos82i = 0;
    c_cos(_sub81r, _sub81i, &_cos82r, &_cos82i);
    double _c83r = 0, _c83i = 0;
    _c83r = 0.0; _c83i = 1.0;
    double _mul84r = 0, _mul84i = 0;
    c_mul(_cos82r, _cos82i, _c83r, _c83i, &_mul84r, &_mul84i);
    double _add85r = 0, _add85i = 0;
    _add85r = _sub80r + _mul84r; _add85i = _sub80i + _mul84i;
    { int _idx = 14; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add85r; cIm[_idx] = _add85i; } }
    double _cf86r = 0, _cf86i = 0;
    { int _idx = 19; if (_idx >= 0 && _idx < 35) { _cf86r = cRe[_idx]; _cf86i = cIm[_idx]; } }
    double _re87r = 0, _re87i = 0;
    _re87r = _cf86r; _re87i = 0;
    double _cf88r = 0, _cf88i = 0;
    { int _idx = 19; if (_idx >= 0 && _idx < 35) { _cf88r = cRe[_idx]; _cf88i = cIm[_idx]; } }
    double _im89r = 0, _im89i = 0;
    _im89r = _cf88i; _im89i = 0;
    double _c90r = 0, _c90i = 0;
    _c90r = 0.0; _c90i = 1.0;
    double _mul91r = 0, _mul91i = 0;
    c_mul(_im89r, _im89i, _c90r, _c90i, &_mul91r, &_mul91i);
    double _add92r = 0, _add92i = 0;
    _add92r = _re87r + _mul91r; _add92i = _re87i + _mul91i;
    double _mul93r = 0, _mul93i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul93r, &_mul93i);
    double _add94r = 0, _add94i = 0;
    _add94r = _add92r + _mul93r; _add94i = _add92i + _mul93i;
    { int _idx = 19; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add94r; cIm[_idx] = _add94i; } }
    double _abs95r = 0, _abs95i = 0;
    _abs95r = c_abs(x1r, x1i); _abs95i = 0;
    double _abs96r = 0, _abs96i = 0;
    _abs96r = c_abs(x2r, x2i); _abs96i = 0;
    double _mul97r = 0, _mul97i = 0;
    c_mul(_abs95r, _abs95i, _abs96r, _abs96i, &_mul97r, &_mul97i);
    double _add98r = 0, _add98i = 0;
    _add98r = x1r + x2r; _add98i = x1i + x2i;
    double _ang99r = 0, _ang99i = 0;
    _ang99r = c_arg(_add98r, _add98i); _ang99i = 0;
    double _c100r = 0, _c100i = 0;
    _c100r = 0.0; _c100i = 1.0;
    double _mul101r = 0, _mul101i = 0;
    c_mul(_ang99r, _ang99i, _c100r, _c100i, &_mul101r, &_mul101i);
    double _add102r = 0, _add102i = 0;
    _add102r = _mul97r + _mul101r; _add102i = _mul97i + _mul101i;
    { int _idx = 24; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add102r; cIm[_idx] = _add102i; } }
    double _c103r = 0, _c103i = 0;
    _c103r = 2.0; _c103i = 0;
    double _pow104r = 0, _pow104i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow104r, &_pow104i);
    double _sin105r = 0, _sin105i = 0;
    c_sin(_pow104r, _pow104i, &_sin105r, &_sin105i);
    double _c106r = 0, _c106i = 0;
    _c106r = 3.0; _c106i = 0;
    double _pow107r = 0, _pow107i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow107r, &_pow107i);
    c_mul(_pow107r, _pow107i, x2r, x2i, &_pow107r, &_pow107i);
    double _cos108r = 0, _cos108i = 0;
    c_cos(_pow107r, _pow107i, &_cos108r, &_cos108i);
    double _c109r = 0, _c109i = 0;
    _c109r = 0.0; _c109i = 1.0;
    double _mul110r = 0, _mul110i = 0;
    c_mul(_cos108r, _cos108i, _c109r, _c109i, &_mul110r, &_mul110i);
    double _add111r = 0, _add111i = 0;
    _add111r = _sin105r + _mul110r; _add111i = _sin105i + _mul110i;
    double _mul112r = 0, _mul112i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul112r, &_mul112i);
    double _abs113r = 0, _abs113i = 0;
    _abs113r = c_abs(_mul112r, _mul112i); _abs113i = 0;
    double _c114r = 0, _c114i = 0;
    _c114r = 1.0; _c114i = 0;
    double _add115r = 0, _add115i = 0;
    _add115r = _abs113r + _c114r; _add115i = _abs113i + _c114i;
    double _log116r = 0, _log116i = 0;
    c_log(_add115r, _add115i, &_log116r, &_log116i);
    double _sub117r = 0, _sub117i = 0;
    _sub117r = _add111r - _log116r; _sub117i = _add111i - _log116i;
    { int _idx = 29; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub117r; cIm[_idx] = _sub117i; } }
    double _cf118r = 0, _cf118i = 0;
    { int _idx = 34; if (_idx >= 0 && _idx < 35) { _cf118r = cRe[_idx]; _cf118i = cIm[_idx]; } }
    double _conj119r = 0, _conj119i = 0;
    _conj119r = _cf118r; _conj119i = -(_cf118i);
    double _add120r = 0, _add120i = 0;
    _add120r = _conj119r + x1r; _add120i = _conj119i + x1i;
    double _sub121r = 0, _sub121i = 0;
    _sub121r = _add120r - x2r; _sub121i = _add120i - x2i;
    double _add122r = 0, _add122i = 0;
    _add122r = x1r + x2r; _add122i = x1i + x2i;
    double _sin123r = 0, _sin123i = 0;
    c_sin(_add122r, _add122i, &_sin123r, &_sin123i);
    double _c124r = 0, _c124i = 0;
    _c124r = 0.0; _c124i = 1.0;
    double _mul125r = 0, _mul125i = 0;
    c_mul(_sin123r, _sin123i, _c124r, _c124i, &_mul125r, &_mul125i);
    double _add126r = 0, _add126i = 0;
    _add126r = _sub121r + _mul125r; _add126i = _sub121i + _mul125i;
    { int _idx = 34; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add126r; cIm[_idx] = _add126i; } }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_239_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 36; j++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 5.0; _c1i = 0;
        double _mod2r = 0, _mod2i = 0;
        _mod2r = fmod(j, _c1r); _mod2i = 0;
        double _c3r = 0, _c3i = 0;
        _c3r = 1.0; _c3i = 0;
        if (_mod2r == _c3r) {
            double _re4r = 0, _re4i = 0;
            _re4r = x1r; _re4i = 0;
            double _pow5r = 0, _pow5i = 0;
            c_powr(_re4r, _re4i, j, &_pow5r, &_pow5i);
            double _im6r = 0, _im6i = 0;
            _im6r = x2i; _im6i = 0;
            double _c7r = 0, _c7i = 0;
            _c7r = 2.0; _c7i = 0;
            double _pow8r = 0, _pow8i = 0;
            c_mul(_im6r, _im6i, _im6r, _im6i, &_pow8r, &_pow8i);
            double _add9r = 0, _add9i = 0;
            _add9r = _pow5r + _pow8r; _add9i = _pow5i + _pow8i;
            { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add9r; cIm[_idx] = _add9i; } }
        } else {
            double _c10r = 0, _c10i = 0;
            _c10r = 5.0; _c10i = 0;
            double _mod11r = 0, _mod11i = 0;
            _mod11r = fmod(j, _c10r); _mod11i = 0;
            double _c12r = 0, _c12i = 0;
            _c12r = 2.0; _c12i = 0;
            if (_mod11r == _c12r) {
                double _abs13r = 0, _abs13i = 0;
                _abs13r = c_abs(x1r, x1i); _abs13i = 0;
                double _abs14r = 0, _abs14i = 0;
                _abs14r = c_abs(x2r, x2i); _abs14i = 0;
                double _pow15r = 0, _pow15i = 0;
                c_powr(_abs14r, _abs14i, j, &_pow15r, &_pow15i);
                double _mul16r = 0, _mul16i = 0;
                c_mul(_abs13r, _abs13i, _pow15r, _pow15i, &_mul16r, &_mul16i);
                double _c17r = 0, _c17i = 0;
                _c17r = 0.0; _c17i = 1.0;
                double _ang18r = 0, _ang18i = 0;
                _ang18r = c_arg(x1r, x1i); _ang18i = 0;
                double _mul19r = 0, _mul19i = 0;
                c_mul(_c17r, _c17i, _ang18r, _ang18i, &_mul19r, &_mul19i);
                double _mul20r = 0, _mul20i = 0;
                c_mul(_mul19r, _mul19i, j, 0, &_mul20r, &_mul20i);
                double _exp21r = 0, _exp21i = 0;
                c_exp2(_mul20r, _mul20i, &_exp21r, &_exp21i);
                double _mul22r = 0, _mul22i = 0;
                c_mul(_mul16r, _mul16i, _exp21r, _exp21i, &_mul22r, &_mul22i);
                { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul22r; cIm[_idx] = _mul22i; } }
            } else {
                double _c23r = 0, _c23i = 0;
                _c23r = 5.0; _c23i = 0;
                double _mod24r = 0, _mod24i = 0;
                _mod24r = fmod(j, _c23r); _mod24i = 0;
                double _c25r = 0, _c25i = 0;
                _c25r = 3.0; _c25i = 0;
                if (_mod24r == _c25r) {
                    double _conj26r = 0, _conj26i = 0;
                    _conj26r = x1r; _conj26i = -(x1i);
                    double _sin27r = 0, _sin27i = 0;
                    c_sin(x2r, x2i, &_sin27r, &_sin27i);
                    double _pow28r = 0, _pow28i = 0;
                    c_powr(_sin27r, _sin27i, j, &_pow28r, &_pow28i);
                    double _mul29r = 0, _mul29i = 0;
                    c_mul(_conj26r, _conj26i, _pow28r, _pow28i, &_mul29r, &_mul29i);
                    double _mul30r = 0, _mul30i = 0;
                    c_mul(x1r, x1i, x2r, x2i, &_mul30r, &_mul30i);
                    double _cos31r = 0, _cos31i = 0;
                    c_cos(_mul30r, _mul30i, &_cos31r, &_cos31i);
                    double _add32r = 0, _add32i = 0;
                    _add32r = _mul29r + _cos31r; _add32i = _mul29i + _cos31i;
                    { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add32r; cIm[_idx] = _add32i; } }
                } else {
                    double _c33r = 0, _c33i = 0;
                    _c33r = 5.0; _c33i = 0;
                    double _mod34r = 0, _mod34i = 0;
                    _mod34r = fmod(j, _c33r); _mod34i = 0;
                    double _c35r = 0, _c35i = 0;
                    _c35r = 4.0; _c35i = 0;
                    if (_mod34r == _c35r) {
                        double _abs36r = 0, _abs36i = 0;
                        _abs36r = c_abs(x1r, x1i); _abs36i = 0;
                        double _c37r = 0, _c37i = 0;
                        _c37r = 1.0; _c37i = 0;
                        double _add38r = 0, _add38i = 0;
                        _add38r = _abs36r + _c37r; _add38i = _abs36i + _c37i;
                        double _log39r = 0, _log39i = 0;
                        c_log(_add38r, _add38i, &_log39r, &_log39i);
                        double _pow40r = 0, _pow40i = 0;
                        c_powr(x2r, x2i, j, &_pow40r, &_pow40i);
                        double _mul41r = 0, _mul41i = 0;
                        c_mul(_log39r, _log39i, _pow40r, _pow40i, &_mul41r, &_mul41i);
                        double _c42r = 0, _c42i = 0;
                        _c42r = 0.0; _c42i = 1.0;
                        double _abs43r = 0, _abs43i = 0;
                        _abs43r = c_abs(x2r, x2i); _abs43i = 0;
                        double _c44r = 0, _c44i = 0;
                        _c44r = 1.0; _c44i = 0;
                        double _add45r = 0, _add45i = 0;
                        _add45r = _abs43r + _c44r; _add45i = _abs43i + _c44i;
                        double _log46r = 0, _log46i = 0;
                        c_log(_add45r, _add45i, &_log46r, &_log46i);
                        double _mul47r = 0, _mul47i = 0;
                        c_mul(_c42r, _c42i, _log46r, _log46i, &_mul47r, &_mul47i);
                        double _add48r = 0, _add48i = 0;
                        _add48r = _mul41r + _mul47r; _add48i = _mul41i + _mul47i;
                        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add48r; cIm[_idx] = _add48i; } }
                    } else {
                        double _add49r = 0, _add49i = 0;
                        _add49r = x1r + x2r; _add49i = x1i + x2i;
                        double _pow50r = 0, _pow50i = 0;
                        c_powr(_add49r, _add49i, j, &_pow50r, &_pow50i);
                        double _sub51r = 0, _sub51i = 0;
                        _sub51r = x1r - x2r; _sub51i = x1i - x2i;
                        double _pow52r = 0, _pow52i = 0;
                        c_powr(_sub51r, _sub51i, j, &_pow52r, &_pow52i);
                        double _sub53r = 0, _sub53i = 0;
                        _sub53r = _pow50r - _pow52r; _sub53i = _pow50i - _pow52i;
                        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub53r; cIm[_idx] = _sub53i; } }
                    }
                }
            }
        }
    }
    double _c54r = 0, _c54i = 0;
    _c54r = 0.0; _c54i = 2.0;
    double _mul55r = 0, _mul55i = 0;
    c_mul(_c54r, _c54i, x1r, x1i, &_mul55r, &_mul55i);
    double _mul56r = 0, _mul56i = 0;
    c_mul(_mul55r, _mul55i, x2r, x2i, &_mul56r, &_mul56i);
    cRe[4] += _mul56r; cIm[4] += _mul56i;
    double _re57r = 0, _re57i = 0;
    _re57r = x1r; _re57i = 0;
    double _c58r = 0, _c58i = 0;
    _c58r = 2.0; _c58i = 0;
    double _pow59r = 0, _pow59i = 0;
    c_mul(_re57r, _re57i, _re57r, _re57i, &_pow59r, &_pow59i);
    double _im60r = 0, _im60i = 0;
    _im60r = x2i; _im60i = 0;
    double _c61r = 0, _c61i = 0;
    _c61r = 3.0; _c61i = 0;
    double _pow62r = 0, _pow62i = 0;
    c_mul(_im60r, _im60i, _im60r, _im60i, &_pow62r, &_pow62i);
    c_mul(_pow62r, _pow62i, _im60r, _im60i, &_pow62r, &_pow62i);
    double _sub63r = 0, _sub63i = 0;
    _sub63r = _pow59r - _pow62r; _sub63i = _pow59i - _pow62i;
    double _c64r = 0, _c64i = 0;
    _c64r = 0.0; _c64i = 3.0;
    double _mul65r = 0, _mul65i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul65r, &_mul65i);
    double _abs66r = 0, _abs66i = 0;
    _abs66r = c_abs(_mul65r, _mul65i); _abs66i = 0;
    double _mul67r = 0, _mul67i = 0;
    c_mul(_c64r, _c64i, _abs66r, _abs66i, &_mul67r, &_mul67i);
    double _add68r = 0, _add68i = 0;
    _add68r = _sub63r + _mul67r; _add68i = _sub63i + _mul67i;
    { int _idx = 9; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add68r; cIm[_idx] = _add68i; } }
    double _add69r = 0, _add69i = 0;
    _add69r = x1r + x2r; _add69i = x1i + x2i;
    double _sin70r = 0, _sin70i = 0;
    c_sin(_add69r, _add69i, &_sin70r, &_sin70i);
    double _sub71r = 0, _sub71i = 0;
    _sub71r = x1r - x2r; _sub71i = x1i - x2i;
    double _cos72r = 0, _cos72i = 0;
    c_cos(_sub71r, _sub71i, &_cos72r, &_cos72i);
    double _mul73r = 0, _mul73i = 0;
    c_mul(_sin70r, _sin70i, _cos72r, _cos72i, &_mul73r, &_mul73i);
    double _c74r = 0, _c74i = 0;
    _c74r = 0.0; _c74i = 1.0;
    double _abs75r = 0, _abs75i = 0;
    _abs75r = c_abs(x1r, x1i); _abs75i = 0;
    double _abs76r = 0, _abs76i = 0;
    _abs76r = c_abs(x2r, x2i); _abs76i = 0;
    double _add77r = 0, _add77i = 0;
    _add77r = _abs75r + _abs76r; _add77i = _abs75i + _abs76i;
    double _c78r = 0, _c78i = 0;
    _c78r = 1.0; _c78i = 0;
    double _add79r = 0, _add79i = 0;
    _add79r = _add77r + _c78r; _add79i = _add77i + _c78i;
    double _log80r = 0, _log80i = 0;
    c_log(_add79r, _add79i, &_log80r, &_log80i);
    double _mul81r = 0, _mul81i = 0;
    c_mul(_c74r, _c74i, _log80r, _log80i, &_mul81r, &_mul81i);
    double _add82r = 0, _add82i = 0;
    _add82r = _mul73r + _mul81r; _add82i = _mul73i + _mul81i;
    { int _idx = 14; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add82r; cIm[_idx] = _add82i; } }
    double _mul83r = 0, _mul83i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul83r, &_mul83i);
    double _c84r = 0, _c84i = 0;
    _c84r = 2.0; _c84i = 0;
    double _pow85r = 0, _pow85i = 0;
    c_mul(_mul83r, _mul83i, _mul83r, _mul83i, &_pow85r, &_pow85i);
    double _conj86r = 0, _conj86i = 0;
    _conj86r = x1r; _conj86i = -(x1i);
    double _conj87r = 0, _conj87i = 0;
    _conj87r = x2r; _conj87i = -(x2i);
    double _mul88r = 0, _mul88i = 0;
    c_mul(_conj86r, _conj86i, _conj87r, _conj87i, &_mul88r, &_mul88i);
    double _sub89r = 0, _sub89i = 0;
    _sub89r = _pow85r - _mul88r; _sub89i = _pow85i - _mul88i;
    double _c90r = 0, _c90i = 0;
    _c90r = 0.0; _c90i = 2.0;
    double _add91r = 0, _add91i = 0;
    _add91r = x1r + x2r; _add91i = x1i + x2i;
    double _ang92r = 0, _ang92i = 0;
    _ang92r = c_arg(_add91r, _add91i); _ang92i = 0;
    double _mul93r = 0, _mul93i = 0;
    c_mul(_c90r, _c90i, _ang92r, _ang92i, &_mul93r, &_mul93i);
    double _add94r = 0, _add94i = 0;
    _add94r = _sub89r + _mul93r; _add94i = _sub89i + _mul93i;
    { int _idx = 19; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add94r; cIm[_idx] = _add94i; } }
    double _mul95r = 0, _mul95i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul95r, &_mul95i);
    double _re96r = 0, _re96i = 0;
    _re96r = _mul95r; _re96i = 0;
    double _im97r = 0, _im97i = 0;
    _im97r = x1i; _im97i = 0;
    double _c98r = 0, _c98i = 0;
    _c98r = 2.0; _c98i = 0;
    double _pow99r = 0, _pow99i = 0;
    c_mul(_im97r, _im97i, _im97r, _im97i, &_pow99r, &_pow99i);
    double _add100r = 0, _add100i = 0;
    _add100r = _re96r + _pow99r; _add100i = _re96i + _pow99i;
    double _im101r = 0, _im101i = 0;
    _im101r = x2i; _im101i = 0;
    double _c102r = 0, _c102i = 0;
    _c102r = 2.0; _c102i = 0;
    double _pow103r = 0, _pow103i = 0;
    c_mul(_im101r, _im101i, _im101r, _im101i, &_pow103r, &_pow103i);
    double _sub104r = 0, _sub104i = 0;
    _sub104r = _add100r - _pow103r; _sub104i = _add100i - _pow103i;
    double _c105r = 0, _c105i = 0;
    _c105r = 0.0; _c105i = 1.0;
    double _re106r = 0, _re106i = 0;
    _re106r = x1r; _re106i = 0;
    double _re107r = 0, _re107i = 0;
    _re107r = x2r; _re107i = 0;
    double _sub108r = 0, _sub108i = 0;
    _sub108r = _re106r - _re107r; _sub108i = _re106i - _re107i;
    double _mul109r = 0, _mul109i = 0;
    c_mul(_c105r, _c105i, _sub108r, _sub108i, &_mul109r, &_mul109i);
    double _add110r = 0, _add110i = 0;
    _add110r = _sub104r + _mul109r; _add110i = _sub104i + _mul109i;
    { int _idx = 24; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add110r; cIm[_idx] = _add110i; } }
    double _add111r = 0, _add111i = 0;
    _add111r = x1r + x2r; _add111i = x1i + x2i;
    double _abs112r = 0, _abs112i = 0;
    _abs112r = c_abs(_add111r, _add111i); _abs112i = 0;
    double _c113r = 0, _c113i = 0;
    _c113r = 3.0; _c113i = 0;
    double _pow114r = 0, _pow114i = 0;
    c_mul(_abs112r, _abs112i, _abs112r, _abs112i, &_pow114r, &_pow114i);
    c_mul(_pow114r, _pow114i, _abs112r, _abs112i, &_pow114r, &_pow114i);
    double _c115r = 0, _c115i = 0;
    _c115r = 0.0; _c115i = 1.0;
    double _sub116r = 0, _sub116i = 0;
    _sub116r = x1r - x2r; _sub116i = x1i - x2i;
    double _ang117r = 0, _ang117i = 0;
    _ang117r = c_arg(_sub116r, _sub116i); _ang117i = 0;
    double _mul118r = 0, _mul118i = 0;
    c_mul(_c115r, _c115i, _ang117r, _ang117i, &_mul118r, &_mul118i);
    double _exp119r = 0, _exp119i = 0;
    c_exp2(_mul118r, _mul118i, &_exp119r, &_exp119i);
    double _mul120r = 0, _mul120i = 0;
    c_mul(_pow114r, _pow114i, _exp119r, _exp119i, &_mul120r, &_mul120i);
    { int _idx = 29; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul120r; cIm[_idx] = _mul120i; } }
    double _abs121r = 0, _abs121i = 0;
    _abs121r = c_abs(x1r, x1i); _abs121i = 0;
    double _mul122r = 0, _mul122i = 0;
    c_mul(_abs121r, _abs121i, x2r, x2i, &_mul122r, &_mul122i);
    double _sin123r = 0, _sin123i = 0;
    c_sin(_mul122r, _mul122i, &_sin123r, &_sin123i);
    double _abs124r = 0, _abs124i = 0;
    _abs124r = c_abs(x2r, x2i); _abs124i = 0;
    double _mul125r = 0, _mul125i = 0;
    c_mul(_abs124r, _abs124i, x1r, x1i, &_mul125r, &_mul125i);
    double _cos126r = 0, _cos126i = 0;
    c_cos(_mul125r, _mul125i, &_cos126r, &_cos126i);
    double _add127r = 0, _add127i = 0;
    _add127r = _sin123r + _cos126r; _add127i = _sin123i + _cos126i;
    double _c128r = 0, _c128i = 0;
    _c128r = 0.0; _c128i = 1.0;
    double _re129r = 0, _re129i = 0;
    _re129r = x1r; _re129i = 0;
    double _re130r = 0, _re130i = 0;
    _re130r = x2r; _re130i = 0;
    double _mul131r = 0, _mul131i = 0;
    c_mul(_re129r, _re129i, _re130r, _re130i, &_mul131r, &_mul131i);
    double _mul132r = 0, _mul132i = 0;
    c_mul(_c128r, _c128i, _mul131r, _mul131i, &_mul132r, &_mul132i);
    double _add133r = 0, _add133i = 0;
    _add133r = _add127r + _mul132r; _add133i = _add127i + _mul132i;
    { int _idx = 34; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add133r; cIm[_idx] = _add133i; } }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_240: auto-stubbed (unhandled constructs in source) */
static void poly_240_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_241: auto-stubbed (unhandled constructs in source) */
static void poly_241_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_242_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 36; j++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 5.0; _c1i = 0;
        double _mod2r = 0, _mod2i = 0;
        _mod2r = fmod(j, _c1r); _mod2i = 0;
        double _c3r = 0, _c3i = 0;
        _c3r = 1.0; _c3i = 0;
        if (_mod2r == _c3r) {
            double _abs4r = 0, _abs4i = 0;
            _abs4r = c_abs(x1r, x1i); _abs4i = 0;
            double _mul5r = 0, _mul5i = 0;
            c_mul(_abs4r, _abs4i, j, 0, &_mul5r, &_mul5i);
            double _sin6r = 0, _sin6i = 0;
            c_sin(_mul5r, _mul5i, &_sin6r, &_sin6i);
            double _ang7r = 0, _ang7i = 0;
            _ang7r = c_arg(x2r, x2i); _ang7i = 0;
            double _mul8r = 0, _mul8i = 0;
            c_mul(_ang7r, _ang7i, j, 0, &_mul8r, &_mul8i);
            double _cos9r = 0, _cos9i = 0;
            c_cos(_mul8r, _mul8i, &_cos9r, &_cos9i);
            double _add10r = 0, _add10i = 0;
            _add10r = _sin6r + _cos9r; _add10i = _sin6i + _cos9i;
            { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add10r; cIm[_idx] = _add10i; } }
        } else {
            double _c11r = 0, _c11i = 0;
            _c11r = 5.0; _c11i = 0;
            double _mod12r = 0, _mod12i = 0;
            _mod12r = fmod(j, _c11r); _mod12i = 0;
            double _c13r = 0, _c13i = 0;
            _c13r = 2.0; _c13i = 0;
            if (_mod12r == _c13r) {
                double _abs14r = 0, _abs14i = 0;
                _abs14r = c_abs(x1r, x1i); _abs14i = 0;
                double _c15r = 0, _c15i = 0;
                _c15r = 1.0; _c15i = 0;
                double _add16r = 0, _add16i = 0;
                _add16r = _abs14r + _c15r; _add16i = _abs14i + _c15i;
                double _log17r = 0, _log17i = 0;
                c_log(_add16r, _add16i, &_log17r, &_log17i);
                double _pow18r = 0, _pow18i = 0;
                c_powr(x2r, x2i, j, &_pow18r, &_pow18i);
                double _mul19r = 0, _mul19i = 0;
                c_mul(_log17r, _log17i, _pow18r, _pow18i, &_mul19r, &_mul19i);
                { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul19r; cIm[_idx] = _mul19i; } }
            } else {
                double _c20r = 0, _c20i = 0;
                _c20r = 5.0; _c20i = 0;
                double _mod21r = 0, _mod21i = 0;
                _mod21r = fmod(j, _c20r); _mod21i = 0;
                double _c22r = 0, _c22i = 0;
                _c22r = 3.0; _c22i = 0;
                if (_mod21r == _c22r) {
                    double _conj23r = 0, _conj23i = 0;
                    _conj23r = x1r; _conj23i = -(x1i);
                    double _pow24r = 0, _pow24i = 0;
                    c_powr(_conj23r, _conj23i, j, &_pow24r, &_pow24i);
                    double _re25r = 0, _re25i = 0;
                    _re25r = x2r; _re25i = 0;
                    double _mul26r = 0, _mul26i = 0;
                    c_mul(_re25r, _re25i, j, 0, &_mul26r, &_mul26i);
                    double _sub27r = 0, _sub27i = 0;
                    _sub27r = _pow24r - _mul26r; _sub27i = _pow24i - _mul26i;
                    { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub27r; cIm[_idx] = _sub27i; } }
                } else {
                    double _c28r = 0, _c28i = 0;
                    _c28r = 5.0; _c28i = 0;
                    double _mod29r = 0, _mod29i = 0;
                    _mod29r = fmod(j, _c28r); _mod29i = 0;
                    double _c30r = 0, _c30i = 0;
                    _c30r = 4.0; _c30i = 0;
                    if (_mod29r == _c30r) {
                        double _im31r = 0, _im31i = 0;
                        _im31r = x1i; _im31i = 0;
                        double _abs32r = 0, _abs32i = 0;
                        _abs32r = c_abs(x2r, x2i); _abs32i = 0;
                        double _ang33r = 0, _ang33i = 0;
                        _ang33r = c_arg(x1r, x1i); _ang33i = 0;
                        double _mul34r = 0, _mul34i = 0;
                        c_mul(j, 0, _ang33r, _ang33i, &_mul34r, &_mul34i);
                        double _sin35r = 0, _sin35i = 0;
                        c_sin(_mul34r, _mul34i, &_sin35r, &_sin35i);
                        double _mul36r = 0, _mul36i = 0;
                        c_mul(_abs32r, _abs32i, _sin35r, _sin35i, &_mul36r, &_mul36i);
                        double _add37r = 0, _add37i = 0;
                        _add37r = _im31r + _mul36r; _add37i = _im31i + _mul36i;
                        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add37r; cIm[_idx] = _add37i; } }
                    } else {
                        double _pow38r = 0, _pow38i = 0;
                        c_powr(x2r, x2i, j, &_pow38r, &_pow38i);
                        double _mul39r = 0, _mul39i = 0;
                        c_mul(x1r, x1i, _pow38r, _pow38i, &_mul39r, &_mul39i);
                        double _cos40r = 0, _cos40i = 0;
                        c_cos(j, 0, &_cos40r, &_cos40i);
                        double _add41r = 0, _add41i = 0;
                        _add41r = _mul39r + _cos40r; _add41i = _mul39i + _cos40i;
                        double _sin42r = 0, _sin42i = 0;
                        c_sin(j, 0, &_sin42r, &_sin42i);
                        double _sub43r = 0, _sub43i = 0;
                        _sub43r = _add41r - _sin42r; _sub43i = _add41i - _sin42i;
                        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub43r; cIm[_idx] = _sub43i; } }
                    }
                }
            }
        }
    }
    double _c44r = 0, _c44i = 0;
    _c44r = 0.0; _c44i = 50.0;
    double _c45r = 0, _c45i = 0;
    _c45r = 2.0; _c45i = 0;
    double _pow46r = 0, _pow46i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow46r, &_pow46i);
    double _mul47r = 0, _mul47i = 0;
    c_mul(_c44r, _c44i, _pow46r, _pow46i, &_mul47r, &_mul47i);
    double _c48r = 0, _c48i = 0;
    _c48r = 0.0; _c48i = 30.0;
    double _mul49r = 0, _mul49i = 0;
    c_mul(_c48r, _c48i, x2r, x2i, &_mul49r, &_mul49i);
    double _sub50r = 0, _sub50i = 0;
    _sub50r = _mul47r - _mul49r; _sub50i = _mul47i - _mul49i;
    double _c51r = 0, _c51i = 0;
    _c51r = 20.0; _c51i = 0;
    double _add52r = 0, _add52i = 0;
    _add52r = _sub50r + _c51r; _add52i = _sub50i + _c51i;
    { int _idx = 6; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add52r; cIm[_idx] = _add52i; } }
    double _c53r = 0, _c53i = 0;
    _c53r = 80.0; _c53i = 0;
    double _mul54r = 0, _mul54i = 0;
    c_mul(_c53r, _c53i, x1r, x1i, &_mul54r, &_mul54i);
    double _c55r = 0, _c55i = 0;
    _c55r = 0.0; _c55i = 60.0;
    double _c56r = 0, _c56i = 0;
    _c56r = 2.0; _c56i = 0;
    double _pow57r = 0, _pow57i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow57r, &_pow57i);
    double _mul58r = 0, _mul58i = 0;
    c_mul(_c55r, _c55i, _pow57r, _pow57i, &_mul58r, &_mul58i);
    double _sub59r = 0, _sub59i = 0;
    _sub59r = _mul54r - _mul58r; _sub59i = _mul54i - _mul58i;
    double _c60r = 0, _c60i = 0;
    _c60r = 10.0; _c60i = 0;
    double _add61r = 0, _add61i = 0;
    _add61r = _sub59r + _c60r; _add61i = _sub59i + _c60i;
    { int _idx = 13; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add61r; cIm[_idx] = _add61i; } }
    double _c62r = 0, _c62i = 0;
    _c62r = 0.0; _c62i = 40.0;
    double _c63r = 0, _c63i = 0;
    _c63r = 3.0; _c63i = 0;
    double _pow64r = 0, _pow64i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow64r, &_pow64i);
    c_mul(_pow64r, _pow64i, x1r, x1i, &_pow64r, &_pow64i);
    double _mul65r = 0, _mul65i = 0;
    c_mul(_c62r, _c62i, _pow64r, _pow64i, &_mul65r, &_mul65i);
    double _c66r = 0, _c66i = 0;
    _c66r = 25.0; _c66i = 0;
    double _conj67r = 0, _conj67i = 0;
    _conj67r = x2r; _conj67i = -(x2i);
    double _mul68r = 0, _mul68i = 0;
    c_mul(_c66r, _c66i, _conj67r, _conj67i, &_mul68r, &_mul68i);
    double _add69r = 0, _add69i = 0;
    _add69r = _mul65r + _mul68r; _add69i = _mul65i + _mul68i;
    double _c70r = 0, _c70i = 0;
    _c70r = 15.0; _c70i = 0;
    double _sub71r = 0, _sub71i = 0;
    _sub71r = _add69r - _c70r; _sub71i = _add69i - _c70i;
    { int _idx = 20; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub71r; cIm[_idx] = _sub71i; } }
    double _c72r = 0, _c72i = 0;
    _c72r = 70.0; _c72i = 0;
    double _abs73r = 0, _abs73i = 0;
    _abs73r = c_abs(x1r, x1i); _abs73i = 0;
    double _mul74r = 0, _mul74i = 0;
    c_mul(_c72r, _c72i, _abs73r, _abs73i, &_mul74r, &_mul74i);
    double _c75r = 0, _c75i = 0;
    _c75r = 0.0; _c75i = 35.0;
    double _ang76r = 0, _ang76i = 0;
    _ang76r = c_arg(x2r, x2i); _ang76i = 0;
    double _mul77r = 0, _mul77i = 0;
    c_mul(_c75r, _c75i, _ang76r, _ang76i, &_mul77r, &_mul77i);
    double _add78r = 0, _add78i = 0;
    _add78r = _mul74r + _mul77r; _add78i = _mul74i + _mul77i;
    double _c79r = 0, _c79i = 0;
    _c79r = 5.0; _c79i = 0;
    double _add80r = 0, _add80i = 0;
    _add80r = _add78r + _c79r; _add80i = _add78i + _c79i;
    { int _idx = 27; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add80r; cIm[_idx] = _add80i; } }
    double _c81r = 0, _c81i = 0;
    _c81r = 0.0; _c81i = 90.0;
    double _mul82r = 0, _mul82i = 0;
    c_mul(_c81r, _c81i, x1r, x1i, &_mul82r, &_mul82i);
    double _mul83r = 0, _mul83i = 0;
    c_mul(_mul82r, _mul82i, x2r, x2i, &_mul83r, &_mul83i);
    double _c84r = 0, _c84i = 0;
    _c84r = 45.0; _c84i = 0;
    double _re85r = 0, _re85i = 0;
    _re85r = x1r; _re85i = 0;
    double _mul86r = 0, _mul86i = 0;
    c_mul(_c84r, _c84i, _re85r, _re85i, &_mul86r, &_mul86i);
    double _sub87r = 0, _sub87i = 0;
    _sub87r = _mul83r - _mul86r; _sub87i = _mul83i - _mul86i;
    double _c88r = 0, _c88i = 0;
    _c88r = 22.5; _c88i = 0;
    double _add89r = 0, _add89i = 0;
    _add89r = _sub87r + _c88r; _add89i = _sub87i + _c88i;
    { int _idx = 34; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add89r; cIm[_idx] = _add89i; } }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_243_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 2.0; _c1i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _c1r; cIm[_idx] = _c1i; } }
    double _c2r = 0, _c2i = 0;
    _c2r = 3.0; _c2i = 0;
    double _neg3r = 0, _neg3i = 0;
    _neg3r = -(_c2r); _neg3i = -(_c2i);
    double _c4r = 0, _c4i = 0;
    _c4r = 0.0; _c4i = 2.0;
    double _add5r = 0, _add5i = 0;
    _add5r = _neg3r + _c4r; _add5i = _neg3i + _c4i;
    { int _idx = 5; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add5r; cIm[_idx] = _add5i; } }
    double _c6r = 0, _c6i = 0;
    _c6r = 4.5; _c6i = 0;
    { int _idx = 9; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _c6r; cIm[_idx] = _c6i; } }
    double _c7r = 0, _c7i = 0;
    _c7r = 0.0; _c7i = 5.2;
    double _neg8r = 0, _neg8i = 0;
    _neg8r = -(_c7r); _neg8i = -(_c7i);
    { int _idx = 14; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _neg8r; cIm[_idx] = _neg8i; } }
    double _c9r = 0, _c9i = 0;
    _c9r = 3.3; _c9i = 0;
    { int _idx = 21; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _c9r; cIm[_idx] = _c9i; } }
    double _c10r = 0, _c10i = 0;
    _c10r = 1.1; _c10i = 0;
    double _neg11r = 0, _neg11i = 0;
    _neg11r = -(_c10r); _neg11i = -(_c10i);
    { int _idx = 27; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _neg11r; cIm[_idx] = _neg11i; } }
    for (int j = 2; j < 6; j++) {
        double _re12r = 0, _re12i = 0;
        _re12r = x1r; _re12i = 0;
        double _pow13r = 0, _pow13i = 0;
        c_powr(_re12r, _re12i, j, &_pow13r, &_pow13i);
        double _im14r = 0, _im14i = 0;
        _im14r = x2i; _im14i = 0;
        double _pow15r = 0, _pow15i = 0;
        c_powr(_im14r, _im14i, j, &_pow15r, &_pow15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _pow13r + _pow15r; _add16i = _pow13i + _pow15i;
        double _ang17r = 0, _ang17i = 0;
        _ang17r = c_arg(x1r, x1i); _ang17i = 0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(_ang17r, _ang17i, j, 0, &_mul18r, &_mul18i);
        double _sin19r = 0, _sin19i = 0;
        c_sin(_mul18r, _mul18i, &_sin19r, &_sin19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_add16r, _add16i, _sin19r, _sin19i, &_mul20r, &_mul20i);
        double _c21r = 0, _c21i = 0;
        _c21r = 1.0; _c21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = _c21r + j; _add22i = _c21i + 0;
        double _div23r = 0, _div23i = 0;
        c_div(_mul20r, _mul20i, _add22r, _add22i, &_div23r, &_div23i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _div23r; cIm[_idx] = _div23i; } }
    }
    for (int k = 7; k < 15; k++) {
        double _abs24r = 0, _abs24i = 0;
        _abs24r = c_abs(x1r, x1i); _abs24i = 0;
        double _pow25r = 0, _pow25i = 0;
        c_powr(_abs24r, _abs24i, k, &_pow25r, &_pow25i);
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(x2r, x2i); _ang26i = 0;
        double _mul27r = 0, _mul27i = 0;
        c_mul(_ang26r, _ang26i, k, 0, &_mul27r, &_mul27i);
        double _cos28r = 0, _cos28i = 0;
        c_cos(_mul27r, _mul27i, &_cos28r, &_cos28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_pow25r, _pow25i, _cos28r, _cos28i, &_mul29r, &_mul29i);
        double _conj30r = 0, _conj30i = 0;
        _conj30r = x2r; _conj30i = -(x2i);
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
        double _mul36r = 0, _mul36i = 0;
        c_mul(_conj30r, _conj30i, _log35r, _log35i, &_mul36r, &_mul36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _mul29r + _mul36r; _add37i = _mul29i + _mul36i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add37r; cIm[_idx] = _add37i; } }
    }
    for (int r = 16; r < 26; r++) {
        double _pow38r = 0, _pow38i = 0;
        c_powr(x1r, x1i, r, &_pow38r, &_pow38i);
        double _re39r = 0, _re39i = 0;
        _re39r = _pow38r; _re39i = 0;
        double _pow40r = 0, _pow40i = 0;
        c_powr(x2r, x2i, r, &_pow40r, &_pow40i);
        double _im41r = 0, _im41i = 0;
        _im41r = _pow40i; _im41i = 0;
        double _c42r = 0, _c42i = 0;
        _c42r = 0.0; _c42i = 1.0;
        double _mul43r = 0, _mul43i = 0;
        c_mul(_im41r, _im41i, _c42r, _c42i, &_mul43r, &_mul43i);
        double _sub44r = 0, _sub44i = 0;
        _sub44r = _re39r - _mul43r; _sub44i = _re39i - _mul43i;
        double _add45r = 0, _add45i = 0;
        _add45r = x1r + x2r; _add45i = x1i + x2i;
        double _sin46r = 0, _sin46i = 0;
        c_sin(_add45r, _add45i, &_sin46r, &_sin46i);
        double _mul47r = 0, _mul47i = 0;
        c_mul(_sub44r, _sub44i, _sin46r, _sin46i, &_mul47r, &_mul47i);
        double _mul48r = 0, _mul48i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul48r, &_mul48i);
        double _cos49r = 0, _cos49i = 0;
        c_cos(_mul48r, _mul48i, &_cos49r, &_cos49i);
        double _add50r = 0, _add50i = 0;
        _add50r = _mul47r + _cos49r; _add50i = _mul47i + _cos49i;
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add50r; cIm[_idx] = _add50i; } }
    }
    double _abs51r = 0, _abs51i = 0;
    _abs51r = c_abs(x1r, x1i); _abs51i = 0;
    double _abs52r = 0, _abs52i = 0;
    _abs52r = c_abs(x2r, x2i); _abs52i = 0;
    double _prod53r = 0, _prod53i = 0;
    c_mul(_abs51r, _abs51i, _abs52r, _abs52i, &_prod53r, &_prod53i);
    double _re54r = 0, _re54i = 0;
    _re54r = x1r; _re54i = 0;
    double _im55r = 0, _im55i = 0;
    _im55r = x2i; _im55i = 0;
    double _sum56r = 0, _sum56i = 0;
    _sum56r = _re54r + _im55r; _sum56i = _re54i + _im55i;
    double _add57r = 0, _add57i = 0;
    _add57r = x1r + x2r; _add57i = x1i + x2i;
    double _conj58r = 0, _conj58i = 0;
    _conj58r = _add57r; _conj58i = -(_add57i);
    double _mul59r = 0, _mul59i = 0;
    c_mul(_sum56r, _sum56i, _conj58r, _conj58i, &_mul59r, &_mul59i);
    double _add60r = 0, _add60i = 0;
    _add60r = _prod53r + _mul59r; _add60i = _prod53i + _mul59i;
    { int _idx = 25; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add60r; cIm[_idx] = _add60i; } }
    double _abs61r = 0, _abs61i = 0;
    _abs61r = c_abs(x1r, x1i); _abs61i = 0;
    double _c62r = 0, _c62i = 0;
    _c62r = 1.0; _c62i = 0;
    double _add63r = 0, _add63i = 0;
    _add63r = _abs61r + _c62r; _add63i = _abs61i + _c62i;
    double _log64r = 0, _log64i = 0;
    c_log(_add63r, _add63i, &_log64r, &_log64i);
    double _abs65r = 0, _abs65i = 0;
    _abs65r = c_abs(x2r, x2i); _abs65i = 0;
    double _c66r = 0, _c66i = 0;
    _c66r = 1.0; _c66i = 0;
    double _add67r = 0, _add67i = 0;
    _add67r = _abs65r + _c66r; _add67i = _abs65i + _c66i;
    double _log68r = 0, _log68i = 0;
    c_log(_add67r, _add67i, &_log68r, &_log68i);
    double _c69r = 0, _c69i = 0;
    _c69r = 0.0; _c69i = 1.0;
    double _mul70r = 0, _mul70i = 0;
    c_mul(_log68r, _log68i, _c69r, _c69i, &_mul70r, &_mul70i);
    double _add71r = 0, _add71i = 0;
    _add71r = _log64r + _mul70r; _add71i = _log64i + _mul70i;
    { int _idx = 26; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add71r; cIm[_idx] = _add71i; } }
    double _mul72r = 0, _mul72i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul72r, &_mul72i);
    double _re73r = 0, _re73i = 0;
    _re73r = _mul72r; _re73i = 0;
    double _div74r = 0, _div74i = 0;
    c_div(x1r, x1i, x2r, x2i, &_div74r, &_div74i);
    double _im75r = 0, _im75i = 0;
    _im75r = _div74i; _im75i = 0;
    double _c76r = 0, _c76i = 0;
    _c76r = 0.0; _c76i = 1.0;
    double _mul77r = 0, _mul77i = 0;
    c_mul(_im75r, _im75i, _c76r, _c76i, &_mul77r, &_mul77i);
    double _sub78r = 0, _sub78i = 0;
    _sub78r = _re73r - _mul77r; _sub78i = _re73i - _mul77i;
    { int _idx = 28; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub78r; cIm[_idx] = _sub78i; } }
    double _c79r = 0, _c79i = 0;
    _c79r = 2.0; _c79i = 0;
    double _pow80r = 0, _pow80i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow80r, &_pow80i);
    double _sin81r = 0, _sin81i = 0;
    c_sin(_pow80r, _pow80i, &_sin81r, &_sin81i);
    double _c82r = 0, _c82i = 0;
    _c82r = 3.0; _c82i = 0;
    double _pow83r = 0, _pow83i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow83r, &_pow83i);
    c_mul(_pow83r, _pow83i, x2r, x2i, &_pow83r, &_pow83i);
    double _cos84r = 0, _cos84i = 0;
    c_cos(_pow83r, _pow83i, &_cos84r, &_cos84i);
    double _c85r = 0, _c85i = 0;
    _c85r = 0.0; _c85i = 1.0;
    double _mul86r = 0, _mul86i = 0;
    c_mul(_cos84r, _cos84i, _c85r, _c85i, &_mul86r, &_mul86i);
    double _add87r = 0, _add87i = 0;
    _add87r = _sin81r + _mul86r; _add87i = _sin81i + _mul86i;
    { int _idx = 29; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add87r; cIm[_idx] = _add87i; } }
    double _add88r = 0, _add88i = 0;
    _add88r = x1r + x2r; _add88i = x1i + x2i;
    double _abs89r = 0, _abs89i = 0;
    _abs89r = c_abs(_add88r, _add88i); _abs89i = 0;
    double _sub90r = 0, _sub90i = 0;
    _sub90r = x1r - x2r; _sub90i = x1i - x2i;
    double _re91r = 0, _re91i = 0;
    _re91r = _sub90r; _re91i = 0;
    double _neg92r = 0, _neg92i = 0;
    _neg92r = -(_re91r); _neg92i = -(_re91i);
    double _exp93r = 0, _exp93i = 0;
    c_exp2(_neg92r, _neg92i, &_exp93r, &_exp93i);
    double _mul94r = 0, _mul94i = 0;
    c_mul(_abs89r, _abs89i, _exp93r, _exp93i, &_mul94r, &_mul94i);
    { int _idx = 31; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul94r; cIm[_idx] = _mul94i; } }
    double _ang95r = 0, _ang95i = 0;
    _ang95r = c_arg(x1r, x1i); _ang95i = 0;
    double _ang96r = 0, _ang96i = 0;
    _ang96r = c_arg(x2r, x2i); _ang96i = 0;
    double _c97r = 0, _c97i = 0;
    _c97r = 0.0; _c97i = 1.0;
    double _mul98r = 0, _mul98i = 0;
    c_mul(_ang96r, _ang96i, _c97r, _c97i, &_mul98r, &_mul98i);
    double _add99r = 0, _add99i = 0;
    _add99r = _ang95r + _mul98r; _add99i = _ang95i + _mul98i;
    { int _idx = 33; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add99r; cIm[_idx] = _add99i; } }
    double _c100r = 0, _c100i = 0;
    _c100r = 3.0; _c100i = 0;
    double _pow101r = 0, _pow101i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow101r, &_pow101i);
    c_mul(_pow101r, _pow101i, x1r, x1i, &_pow101r, &_pow101i);
    double _c102r = 0, _c102i = 0;
    _c102r = 3.0; _c102i = 0;
    double _pow103r = 0, _pow103i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow103r, &_pow103i);
    c_mul(_pow103r, _pow103i, x2r, x2i, &_pow103r, &_pow103i);
    double _add104r = 0, _add104i = 0;
    _add104r = _pow101r + _pow103r; _add104i = _pow101i + _pow103i;
    double _c105r = 0, _c105i = 0;
    _c105r = 1.0; _c105i = 0;
    double _abs106r = 0, _abs106i = 0;
    _abs106r = c_abs(x1r, x1i); _abs106i = 0;
    double _add107r = 0, _add107i = 0;
    _add107r = _c105r + _abs106r; _add107i = _c105i + _abs106i;
    double _abs108r = 0, _abs108i = 0;
    _abs108r = c_abs(x2r, x2i); _abs108i = 0;
    double _add109r = 0, _add109i = 0;
    _add109r = _add107r + _abs108r; _add109i = _add107i + _abs108i;
    double _div110r = 0, _div110i = 0;
    c_div(_add104r, _add104i, _add109r, _add109i, &_div110r, &_div110i);
    { int _idx = 34; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _div110r; cIm[_idx] = _div110i; } }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_244_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 36; j++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 6.0; _c1i = 0;
        double _mod2r = 0, _mod2i = 0;
        _mod2r = fmod(j, _c1r); _mod2i = 0;
        double _c3r = 0, _c3i = 0;
        _c3r = 1.0; _c3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = _mod2r + _c3r; _add4i = _mod2i + _c3i;
        double k_r = _add4r, k_i = _add4i;
        double _c5r = 0, _c5i = 0;
        _c5r = 4.0; _c5i = 0;
        double _mod6r = 0, _mod6i = 0;
        _mod6r = fmod(j, _c5r); _mod6i = 0;
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _mod6r + _c7r; _add8i = _mod6i + _c7i;
        double r_r = _add8r, r_i = _add8i;
        double _re9r = 0, _re9i = 0;
        _re9r = x1r; _re9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(j, 0, _re9r, _re9i, &_mul10r, &_mul10i);
        double _sin11r = 0, _sin11i = 0;
        c_sin(_mul10r, _mul10i, &_sin11r, &_sin11i);
        double _im12r = 0, _im12i = 0;
        _im12r = x2i; _im12i = 0;
        double _mul13r = 0, _mul13i = 0;
        c_mul(j, 0, _im12r, _im12i, &_mul13r, &_mul13i);
        double _cos14r = 0, _cos14i = 0;
        c_cos(_mul13r, _mul13i, &_cos14r, &_cos14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_sin11r, _sin11i, _cos14r, _cos14i, &_mul15r, &_mul15i);
        double _ang16r = 0, _ang16i = 0;
        _ang16r = c_arg(x1r, x1i); _ang16i = 0;
        double _c17r = 0, _c17i = 0;
        _c17r = 1.0; _c17i = 0;
        double _add18r = 0, _add18i = 0;
        _add18r = k_r + _c17r; _add18i = k_i + _c17i;
        double _div19r = 0, _div19i = 0;
        c_div(_ang16r, _ang16i, _add18r, _add18i, &_div19r, &_div19i);
        double _add20r = 0, _add20i = 0;
        _add20r = _mul15r + _div19r; _add20i = _mul15i + _div19i;
        double angle_part_r = _add20r, angle_part_i = _add20i;
        double _abs21r = 0, _abs21i = 0;
        _abs21r = c_abs(x1r, x1i); _abs21i = 0;
        double _pow22r = 0, _pow22i = 0;
        c_powc(_abs21r, _abs21i, k_r, k_i, &_pow22r, &_pow22i);
        double _abs23r = 0, _abs23i = 0;
        _abs23r = c_abs(x2r, x2i); _abs23i = 0;
        double _pow24r = 0, _pow24i = 0;
        c_powc(_abs23r, _abs23i, r_r, r_i, &_pow24r, &_pow24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_pow22r, _pow22i, _pow24r, _pow24i, &_mul25r, &_mul25i);
        double _abs26r = 0, _abs26i = 0;
        _abs26r = c_abs(x1r, x1i); _abs26i = 0;
        double _abs27r = 0, _abs27i = 0;
        _abs27r = c_abs(x2r, x2i); _abs27i = 0;
        double _add28r = 0, _add28i = 0;
        _add28r = _abs26r + _abs27r; _add28i = _abs26i + _abs27i;
        double _add29r = 0, _add29i = 0;
        _add29r = _add28r + j; _add29i = _add28i + 0;
        double _log30r = 0, _log30i = 0;
        c_log(_add29r, _add29i, &_log30r, &_log30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _mul25r + _log30r; _add31i = _mul25i + _log30i;
        double mag_part_r = _add31r, mag_part_i = _add31i;
        double _cos32r = 0, _cos32i = 0;
        c_cos(angle_part_r, angle_part_i, &_cos32r, &_cos32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_cos32r, _cos32i, mag_part_r, mag_part_i, &_mul33r, &_mul33i);
        double _sin34r = 0, _sin34i = 0;
        c_sin(angle_part_r, angle_part_i, &_sin34r, &_sin34i);
        double _mul35r = 0, _mul35i = 0;
        c_mul(_sin34r, _sin34i, mag_part_r, mag_part_i, &_mul35r, &_mul35i);
        double _c36r = 0, _c36i = 0;
        _c36r = 0.0; _c36i = 1.0;
        double _mul37r = 0, _mul37i = 0;
        c_mul(_mul35r, _mul35i, _c36r, _c36i, &_mul37r, &_mul37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _mul33r + _mul37r; _add38i = _mul33i + _mul37i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add38r; cIm[_idx] = _add38i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_245: auto-stubbed (unhandled constructs in source) */
static void poly_245_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_246: auto-stubbed (unhandled constructs in source) */
static void poly_246_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_247_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 36; j++) {
        double _ang1r = 0, _ang1i = 0;
        _ang1r = c_arg(x1r, x1i); _ang1i = 0;
        double _pow2r = 0, _pow2i = 0;
        c_powr(_ang1r, _ang1i, j, &_pow2r, &_pow2i);
        double _ang3r = 0, _ang3i = 0;
        _ang3r = c_arg(x2r, x2i); _ang3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 5.0; _c4i = 0;
        double _mod5r = 0, _mod5i = 0;
        _mod5r = fmod(j, _c4r); _mod5i = 0;
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = _mod5r + _c6r; _add7i = _mod5i + _c6i;
        double _pow8r = 0, _pow8i = 0;
        c_powc(_ang3r, _ang3i, _add7r, _add7i, &_pow8r, &_pow8i);
        double _add9r = 0, _add9i = 0;
        _add9r = _pow2r + _pow8r; _add9i = _pow2i + _pow8i;
        double angle_r = _add9r, angle_i = _add9i;
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(x1r, x1i); _abs10i = 0;
        double _c11r = 0, _c11i = 0;
        _c11r = 7.0; _c11i = 0;
        double _mod12r = 0, _mod12i = 0;
        _mod12r = fmod(j, _c11r); _mod12i = 0;
        double _pow13r = 0, _pow13i = 0;
        c_powc(_abs10r, _abs10i, _mod12r, _mod12i, &_pow13r, &_pow13i);
        double _abs14r = 0, _abs14i = 0;
        _abs14r = c_abs(x2r, x2i); _abs14i = 0;
        double _c15r = 0, _c15i = 0;
        _c15r = 5.0; _c15i = 0;
        double _fdiv16r = 0, _fdiv16i = 0;
        c_div(j, 0, _c15r, _c15i, &_fdiv16r, &_fdiv16i);
        _fdiv16r = floor(_fdiv16r); _fdiv16i = 0;
        double _c17r = 0, _c17i = 0;
        _c17r = 1.0; _c17i = 0;
        double _add18r = 0, _add18i = 0;
        _add18r = _fdiv16r + _c17r; _add18i = _fdiv16i + _c17i;
        double _pow19r = 0, _pow19i = 0;
        c_powc(_abs14r, _abs14i, _add18r, _add18i, &_pow19r, &_pow19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_pow13r, _pow13i, _pow19r, _pow19i, &_mul20r, &_mul20i);
        double magnitude_r = _mul20r, magnitude_i = _mul20i;
        double _cos21r = 0, _cos21i = 0;
        c_cos(angle_r, angle_i, &_cos21r, &_cos21i);
        double _c22r = 0, _c22i = 0;
        _c22r = 0.0; _c22i = 1.0;
        double _sin23r = 0, _sin23i = 0;
        c_sin(angle_r, angle_i, &_sin23r, &_sin23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_c22r, _c22i, _sin23r, _sin23i, &_mul24r, &_mul24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _cos21r + _mul24r; _add25i = _cos21i + _mul24i;
        double _mul26r = 0, _mul26i = 0;
        c_mul(magnitude_r, magnitude_i, _add25r, _add25i, &_mul26r, &_mul26i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul26r; cIm[_idx] = _mul26i; } }
    }
    double k = 1.0;
    for (int r = 2; r < 35; r += 3) {
        double _mul27r = 0, _mul27i = 0;
        c_mul(x1r, x1i, r, 0, &_mul27r, &_mul27i);
        double _sin28r = 0, _sin28i = 0;
        c_sin(_mul27r, _mul27i, &_sin28r, &_sin28i);
        double _c29r = 0, _c29i = 0;
        _c29r = 1.0; _c29i = 0;
        double _add30r = 0, _add30i = 0;
        _add30r = r + _c29r; _add30i = 0 + _c29i;
        double _div31r = 0, _div31i = 0;
        c_div(x2r, x2i, _add30r, _add30i, &_div31r, &_div31i);
        double _cos32r = 0, _cos32i = 0;
        c_cos(_div31r, _div31i, &_cos32r, &_cos32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _sin28r + _cos32r; _add33i = _sin28i + _cos32i;
        { double _tr = cRe[(r - 1)]*_add33r - cIm[(r - 1)]*_add33i; cIm[(r - 1)] = cRe[(r - 1)]*_add33i + cIm[(r - 1)]*_add33r; cRe[(r - 1)] = _tr; }
    }
    double _abs34r = 0, _abs34i = 0;
    _abs34r = c_abs(x1r, x1i); _abs34i = 0;
    double _abs35r = 0, _abs35i = 0;
    _abs35r = c_abs(x2r, x2i); _abs35i = 0;
    double _sum36r = 0, _sum36i = 0;
    _sum36r = _abs34r + _abs35r; _sum36i = _abs34i + _abs35i;
    double _c37r = 0, _c37i = 0;
    _c37r = 0.0; _c37i = 1.0;
    double _add38r = 0, _add38i = 0;
    _add38r = x1r + x2r; _add38i = x1i + x2i;
    double _ang39r = 0, _ang39i = 0;
    _ang39r = c_arg(_add38r, _add38i); _ang39i = 0;
    double _mul40r = 0, _mul40i = 0;
    c_mul(_c37r, _c37i, _ang39r, _ang39i, &_mul40r, &_mul40i);
    double _exp41r = 0, _exp41i = 0;
    c_exp2(_mul40r, _mul40i, &_exp41r, &_exp41i);
    double _mul42r = 0, _mul42i = 0;
    c_mul(_sum36r, _sum36i, _exp41r, _exp41i, &_mul42r, &_mul42i);
    { int _idx = 9; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul42r; cIm[_idx] = _mul42i; } }
    double _abs43r = 0, _abs43i = 0;
    _abs43r = c_abs(x1r, x1i); _abs43i = 0;
    double _abs44r = 0, _abs44i = 0;
    _abs44r = c_abs(x2r, x2i); _abs44i = 0;
    double _prod45r = 0, _prod45i = 0;
    c_mul(_abs43r, _abs43i, _abs44r, _abs44i, &_prod45r, &_prod45i);
    double _c46r = 0, _c46i = 0;
    _c46r = 1.0; _c46i = 0;
    double _sub47r = 0, _sub47i = 0;
    _sub47r = x1r - x2r; _sub47i = x1i - x2i;
    double _abs48r = 0, _abs48i = 0;
    _abs48r = c_abs(_sub47r, _sub47i); _abs48i = 0;
    double _add49r = 0, _add49i = 0;
    _add49r = _c46r + _abs48r; _add49i = _c46i + _abs48i;
    double _div50r = 0, _div50i = 0;
    c_div(_prod45r, _prod45i, _add49r, _add49i, &_div50r, &_div50i);
    { int _idx = 19; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _div50r; cIm[_idx] = _div50i; } }
    double _re51r = 0, _re51i = 0;
    _re51r = x1r; _re51i = 0;
    double _c52r = 0, _c52i = 0;
    _c52r = 3.0; _c52i = 0;
    double _pow53r = 0, _pow53i = 0;
    c_mul(_re51r, _re51i, _re51r, _re51i, &_pow53r, &_pow53i);
    c_mul(_pow53r, _pow53i, _re51r, _re51i, &_pow53r, &_pow53i);
    double _im54r = 0, _im54i = 0;
    _im54r = x2i; _im54i = 0;
    double _c55r = 0, _c55i = 0;
    _c55r = 2.0; _c55i = 0;
    double _pow56r = 0, _pow56i = 0;
    c_mul(_im54r, _im54i, _im54r, _im54i, &_pow56r, &_pow56i);
    double _sub57r = 0, _sub57i = 0;
    _sub57r = _pow53r - _pow56r; _sub57i = _pow53i - _pow56i;
    double _c58r = 0, _c58i = 0;
    _c58r = 0.0; _c58i = 2.0;
    double _re59r = 0, _re59i = 0;
    _re59r = x2r; _re59i = 0;
    double _mul60r = 0, _mul60i = 0;
    c_mul(_c58r, _c58i, _re59r, _re59i, &_mul60r, &_mul60i);
    double _im61r = 0, _im61i = 0;
    _im61r = x1i; _im61i = 0;
    double _mul62r = 0, _mul62i = 0;
    c_mul(_mul60r, _mul60i, _im61r, _im61i, &_mul62r, &_mul62i);
    double _add63r = 0, _add63i = 0;
    _add63r = _sub57r + _mul62r; _add63i = _sub57i + _mul62i;
    { int _idx = 34; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add63r; cIm[_idx] = _add63i; } }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_248: auto-stubbed (unhandled constructs in source) */
static void poly_248_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_249_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 36; j++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 5.0; _c1i = 0;
        double _mod2r = 0, _mod2i = 0;
        _mod2r = fmod(j, _c1r); _mod2i = 0;
        double _c3r = 0, _c3i = 0;
        _c3r = 1.0; _c3i = 0;
        if (_mod2r == _c3r) {
            double _re4r = 0, _re4i = 0;
            _re4r = x1r; _re4i = 0;
            double _pow5r = 0, _pow5i = 0;
            c_powr(_re4r, _re4i, j, &_pow5r, &_pow5i);
            double _im6r = 0, _im6i = 0;
            _im6r = x2i; _im6i = 0;
            double _c7r = 0, _c7i = 0;
            _c7r = 3.0; _c7i = 0;
            double _mod8r = 0, _mod8i = 0;
            _mod8r = fmod(j, _c7r); _mod8i = 0;
            double _pow9r = 0, _pow9i = 0;
            c_powc(_im6r, _im6i, _mod8r, _mod8i, &_pow9r, &_pow9i);
            double _ang10r = 0, _ang10i = 0;
            _ang10r = c_arg(x1r, x1i); _ang10i = 0;
            double _mul11r = 0, _mul11i = 0;
            c_mul(j, 0, _ang10r, _ang10i, &_mul11r, &_mul11i);
            double _sin12r = 0, _sin12i = 0;
            c_sin(_mul11r, _mul11i, &_sin12r, &_sin12i);
            double _mul13r = 0, _mul13i = 0;
            c_mul(_pow9r, _pow9i, _sin12r, _sin12i, &_mul13r, &_mul13i);
            double _add14r = 0, _add14i = 0;
            _add14r = _pow5r + _mul13r; _add14i = _pow5i + _mul13i;
            { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add14r; cIm[_idx] = _add14i; } }
        } else {
            double _c15r = 0, _c15i = 0;
            _c15r = 5.0; _c15i = 0;
            double _mod16r = 0, _mod16i = 0;
            _mod16r = fmod(j, _c15r); _mod16i = 0;
            double _c17r = 0, _c17i = 0;
            _c17r = 2.0; _c17i = 0;
            if (_mod16r == _c17r) {
                double _conj18r = 0, _conj18i = 0;
                _conj18r = x1r; _conj18i = -(x1i);
                double _ang19r = 0, _ang19i = 0;
                _ang19r = c_arg(x2r, x2i); _ang19i = 0;
                double _mul20r = 0, _mul20i = 0;
                c_mul(j, 0, _ang19r, _ang19i, &_mul20r, &_mul20i);
                double _cos21r = 0, _cos21i = 0;
                c_cos(_mul20r, _mul20i, &_cos21r, &_cos21i);
                double _mul22r = 0, _mul22i = 0;
                c_mul(_conj18r, _conj18i, _cos21r, _cos21i, &_mul22r, &_mul22i);
                double _abs23r = 0, _abs23i = 0;
                _abs23r = c_abs(x2r, x2i); _abs23i = 0;
                double _c24r = 0, _c24i = 0;
                _c24r = 2.0; _c24i = 0;
                double _pow25r = 0, _pow25i = 0;
                c_mul(_abs23r, _abs23i, _abs23r, _abs23i, &_pow25r, &_pow25i);
                double _c26r = 0, _c26i = 0;
                _c26r = 1.0; _c26i = 0;
                double _add27r = 0, _add27i = 0;
                _add27r = j + _c26r; _add27i = 0 + _c26i;
                double _div28r = 0, _div28i = 0;
                c_div(_pow25r, _pow25i, _add27r, _add27i, &_div28r, &_div28i);
                double _add29r = 0, _add29i = 0;
                _add29r = _mul22r + _div28r; _add29i = _mul22i + _div28i;
                { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add29r; cIm[_idx] = _add29i; } }
            } else {
                double _c30r = 0, _c30i = 0;
                _c30r = 5.0; _c30i = 0;
                double _mod31r = 0, _mod31i = 0;
                _mod31r = fmod(j, _c30r); _mod31i = 0;
                double _c32r = 0, _c32i = 0;
                _c32r = 3.0; _c32i = 0;
                if (_mod31r == _c32r) {
                    double _abs33r = 0, _abs33i = 0;
                    _abs33r = c_abs(x1r, x1i); _abs33i = 0;
                    double _c34r = 0, _c34i = 0;
                    _c34r = 1.0; _c34i = 0;
                    double _add35r = 0, _add35i = 0;
                    _add35r = _abs33r + _c34r; _add35i = _abs33i + _c34i;
                    double _log36r = 0, _log36i = 0;
                    c_log(_add35r, _add35i, &_log36r, &_log36i);
                    double _c37r = 0, _c37i = 0;
                    _c37r = 0.0; _c37i = 1.0;
                    double _abs38r = 0, _abs38i = 0;
                    _abs38r = c_abs(x2r, x2i); _abs38i = 0;
                    double _c39r = 0, _c39i = 0;
                    _c39r = 1.0; _c39i = 0;
                    double _add40r = 0, _add40i = 0;
                    _add40r = _abs38r + _c39r; _add40i = _abs38i + _c39i;
                    double _log41r = 0, _log41i = 0;
                    c_log(_add40r, _add40i, &_log41r, &_log41i);
                    double _mul42r = 0, _mul42i = 0;
                    c_mul(_c37r, _c37i, _log41r, _log41i, &_mul42r, &_mul42i);
                    double _add43r = 0, _add43i = 0;
                    _add43r = _log36r + _mul42r; _add43i = _log36i + _mul42i;
                    double _re44r = 0, _re44i = 0;
                    _re44r = x1r; _re44i = 0;
                    double _c45r = 0, _c45i = 0;
                    _c45r = 2.0; _c45i = 0;
                    double _pow46r = 0, _pow46i = 0;
                    c_mul(_re44r, _re44i, _re44r, _re44i, &_pow46r, &_pow46i);
                    double _add47r = 0, _add47i = 0;
                    _add47r = _add43r + _pow46r; _add47i = _add43i + _pow46i;
                    double _im48r = 0, _im48i = 0;
                    _im48r = x2i; _im48i = 0;
                    double _c49r = 0, _c49i = 0;
                    _c49r = 2.0; _c49i = 0;
                    double _pow50r = 0, _pow50i = 0;
                    c_mul(_im48r, _im48i, _im48r, _im48i, &_pow50r, &_pow50i);
                    double _sub51r = 0, _sub51i = 0;
                    _sub51r = _add47r - _pow50r; _sub51i = _add47i - _pow50i;
                    { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub51r; cIm[_idx] = _sub51i; } }
                } else {
                    double _c52r = 0, _c52i = 0;
                    _c52r = 5.0; _c52i = 0;
                    double _mod53r = 0, _mod53i = 0;
                    _mod53r = fmod(j, _c52r); _mod53i = 0;
                    double _c54r = 0, _c54i = 0;
                    _c54r = 4.0; _c54i = 0;
                    if (_mod53r == _c54r) {
                        double _re55r = 0, _re55i = 0;
                        _re55r = x1r; _re55i = 0;
                        double _im56r = 0, _im56i = 0;
                        _im56r = x2i; _im56i = 0;
                        double _mul57r = 0, _mul57i = 0;
                        c_mul(_re55r, _re55i, _im56r, _im56i, &_mul57r, &_mul57i);
                        double _pow58r = 0, _pow58i = 0;
                        c_powr(_mul57r, _mul57i, j, &_pow58r, &_pow58i);
                        double _abs59r = 0, _abs59i = 0;
                        _abs59r = c_abs(x1r, x1i); _abs59i = 0;
                        double _abs60r = 0, _abs60i = 0;
                        _abs60r = c_abs(x2r, x2i); _abs60i = 0;
                        double _add61r = 0, _add61i = 0;
                        _add61r = _abs59r + _abs60r; _add61i = _abs59i + _abs60i;
                        double _sin62r = 0, _sin62i = 0;
                        c_sin(j, 0, &_sin62r, &_sin62i);
                        double _mul63r = 0, _mul63i = 0;
                        c_mul(_add61r, _add61i, _sin62r, _sin62i, &_mul63r, &_mul63i);
                        double _add64r = 0, _add64i = 0;
                        _add64r = _pow58r + _mul63r; _add64i = _pow58i + _mul63i;
                        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add64r; cIm[_idx] = _add64i; } }
                    } else {
                        double _re65r = 0, _re65i = 0;
                        _re65r = x1r; _re65i = 0;
                        double _im66r = 0, _im66i = 0;
                        _im66r = x2i; _im66i = 0;
                        double _sum67r = 0, _sum67i = 0;
                        _sum67r = _re65r + _im66r; _sum67i = _re65i + _im66i;
                        double _ang68r = 0, _ang68i = 0;
                        _ang68r = c_arg(x1r, x1i); _ang68i = 0;
                        double _mul69r = 0, _mul69i = 0;
                        c_mul(j, 0, _ang68r, _ang68i, &_mul69r, &_mul69i);
                        double _ang70r = 0, _ang70i = 0;
                        _ang70r = c_arg(x2r, x2i); _ang70i = 0;
                        double _mul71r = 0, _mul71i = 0;
                        c_mul(_mul69r, _mul69i, _ang70r, _ang70i, &_mul71r, &_mul71i);
                        double _cos72r = 0, _cos72i = 0;
                        c_cos(_mul71r, _mul71i, &_cos72r, &_cos72i);
                        double _mul73r = 0, _mul73i = 0;
                        c_mul(_sum67r, _sum67i, _cos72r, _cos72i, &_mul73r, &_mul73i);
                        double _c74r = 0, _c74i = 0;
                        _c74r = 0.0; _c74i = 1.0;
                        double _abs75r = 0, _abs75i = 0;
                        _abs75r = c_abs(x1r, x1i); _abs75i = 0;
                        double _abs76r = 0, _abs76i = 0;
                        _abs76r = c_abs(x2r, x2i); _abs76i = 0;
                        double _prod77r = 0, _prod77i = 0;
                        c_mul(_abs75r, _abs75i, _abs76r, _abs76i, &_prod77r, &_prod77i);
                        double _mul78r = 0, _mul78i = 0;
                        c_mul(_c74r, _c74i, _prod77r, _prod77i, &_mul78r, &_mul78i);
                        double _add79r = 0, _add79i = 0;
                        _add79r = _mul73r + _mul78r; _add79i = _mul73i + _mul78i;
                        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add79r; cIm[_idx] = _add79i; } }
                    }
                }
            }
        }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_250: auto-stubbed (unhandled constructs in source) */
static void poly_250_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_251_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 36; j++) {
        double _re1r = 0, _re1i = 0;
        _re1r = x1r; _re1i = 0;
        double _re2r = 0, _re2i = 0;
        _re2r = x2r; _re2i = 0;
        double _add3r = 0, _add3i = 0;
        _add3r = _re1r + _re2r; _add3i = _re1i + _re2i;
        double _add4r = 0, _add4i = 0;
        _add4r = _add3r + j; _add4i = _add3i + 0;
        double r_r = _add4r, r_i = _add4i;
        double _ang5r = 0, _ang5i = 0;
        _ang5r = c_arg(x1r, x1i); _ang5i = 0;
        double _mul6r = 0, _mul6i = 0;
        c_mul(_ang5r, _ang5i, j, 0, &_mul6r, &_mul6i);
        double _ang7r = 0, _ang7i = 0;
        _ang7r = c_arg(x2r, x2i); _ang7i = 0;
        double _sub8r = 0, _sub8i = 0;
        _sub8r = _mul6r - _ang7r; _sub8i = _mul6i - _ang7i;
        double angle_r = _sub8r, angle_i = _sub8i;
        double _abs9r = 0, _abs9i = 0;
        _abs9r = c_abs(x1r, x1i); _abs9i = 0;
        double _pow10r = 0, _pow10i = 0;
        c_powr(_abs9r, _abs9i, j, &_pow10r, &_pow10i);
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(x2r, x2i); _abs11i = 0;
        double _c12r = 0, _c12i = 0;
        _c12r = 35.0; _c12i = 0;
        double _sub13r = 0, _sub13i = 0;
        _sub13r = _c12r - j; _sub13i = _c12i - 0;
        double _pow14r = 0, _pow14i = 0;
        c_powc(_abs11r, _abs11i, _sub13r, _sub13i, &_pow14r, &_pow14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _pow10r + _pow14r; _add15i = _pow10i + _pow14i;
        double _c16r = 0, _c16i = 0;
        _c16r = 0.0; _c16i = 1.0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(_c16r, _c16i, angle_r, angle_i, &_mul17r, &_mul17i);
        double _exp18r = 0, _exp18i = 0;
        c_exp2(_mul17r, _mul17i, &_exp18r, &_exp18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_add15r, _add15i, _exp18r, _exp18i, &_mul19r, &_mul19i);
        double _re20r = 0, _re20i = 0;
        _re20r = x1r; _re20i = 0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(j, 0, _re20r, _re20i, &_mul21r, &_mul21i);
        double _im22r = 0, _im22i = 0;
        _im22r = x2i; _im22i = 0;
        double _sub23r = 0, _sub23i = 0;
        _sub23r = _mul21r - _im22r; _sub23i = _mul21i - _im22i;
        double _sin24r = 0, _sin24i = 0;
        c_sin(_sub23r, _sub23i, &_sin24r, &_sin24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_mul19r, _mul19i, _sin24r, _sin24i, &_mul25r, &_mul25i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul25r; cIm[_idx] = _mul25i; } }
    }
    double _conj26r = 0, _conj26i = 0;
    _conj26r = x1r; _conj26i = -(x1i);
    double _c27r = 0, _c27i = 0;
    _c27r = 2.0; _c27i = 0;
    double _pow28r = 0, _pow28i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow28r, &_pow28i);
    double _mul29r = 0, _mul29i = 0;
    c_mul(_conj26r, _conj26i, _pow28r, _pow28i, &_mul29r, &_mul29i);
    double _abs30r = 0, _abs30i = 0;
    _abs30r = c_abs(x1r, x1i); _abs30i = 0;
    double _c31r = 0, _c31i = 0;
    _c31r = 1.0; _c31i = 0;
    double _add32r = 0, _add32i = 0;
    _add32r = _abs30r + _c31r; _add32i = _abs30i + _c31i;
    double _log33r = 0, _log33i = 0;
    c_log(_add32r, _add32i, &_log33r, &_log33i);
    double _sub34r = 0, _sub34i = 0;
    _sub34r = _mul29r - _log33r; _sub34i = _mul29i - _log33i;
    double _c35r = 0, _c35i = 0;
    _c35r = 0.0; _c35i = 2.0;
    double _re36r = 0, _re36i = 0;
    _re36r = x2r; _re36i = 0;
    double _mul37r = 0, _mul37i = 0;
    c_mul(_c35r, _c35i, _re36r, _re36i, &_mul37r, &_mul37i);
    double _add38r = 0, _add38i = 0;
    _add38r = _sub34r + _mul37r; _add38i = _sub34i + _mul37i;
    { int _idx = 4; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add38r; cIm[_idx] = _add38i; } }
    double _sin39r = 0, _sin39i = 0;
    c_sin(x1r, x1i, &_sin39r, &_sin39i);
    double _cos40r = 0, _cos40i = 0;
    c_cos(x2r, x2i, &_cos40r, &_cos40i);
    double _conj41r = 0, _conj41i = 0;
    _conj41r = x1r; _conj41i = -(x1i);
    double _mul42r = 0, _mul42i = 0;
    c_mul(_cos40r, _cos40i, _conj41r, _conj41i, &_mul42r, &_mul42i);
    double _add43r = 0, _add43i = 0;
    _add43r = _sin39r + _mul42r; _add43i = _sin39i + _mul42i;
    { int _idx = 9; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add43r; cIm[_idx] = _add43i; } }
    double _mul44r = 0, _mul44i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul44r, &_mul44i);
    double _c45r = 0, _c45i = 0;
    _c45r = 3.0; _c45i = 0;
    double _pow46r = 0, _pow46i = 0;
    c_mul(_mul44r, _mul44i, _mul44r, _mul44i, &_pow46r, &_pow46i);
    c_mul(_pow46r, _pow46i, _mul44r, _mul44i, &_pow46r, &_pow46i);
    double _re47r = 0, _re47i = 0;
    _re47r = x1r; _re47i = 0;
    double _c48r = 0, _c48i = 0;
    _c48r = 2.0; _c48i = 0;
    double _pow49r = 0, _pow49i = 0;
    c_mul(_re47r, _re47i, _re47r, _re47i, &_pow49r, &_pow49i);
    double _sub50r = 0, _sub50i = 0;
    _sub50r = _pow46r - _pow49r; _sub50i = _pow46i - _pow49i;
    double _im51r = 0, _im51i = 0;
    _im51r = x2i; _im51i = 0;
    double _c52r = 0, _c52i = 0;
    _c52r = 3.0; _c52i = 0;
    double _pow53r = 0, _pow53i = 0;
    c_mul(_im51r, _im51i, _im51r, _im51i, &_pow53r, &_pow53i);
    c_mul(_pow53r, _pow53i, _im51r, _im51i, &_pow53r, &_pow53i);
    double _add54r = 0, _add54i = 0;
    _add54r = _sub50r + _pow53r; _add54i = _sub50i + _pow53i;
    { int _idx = 14; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add54r; cIm[_idx] = _add54i; } }
    double _c55r = 0, _c55i = 0;
    _c55r = 0.0; _c55i = 1.0;
    double _ang56r = 0, _ang56i = 0;
    _ang56r = c_arg(x1r, x1i); _ang56i = 0;
    double _mul57r = 0, _mul57i = 0;
    c_mul(_c55r, _c55i, _ang56r, _ang56i, &_mul57r, &_mul57i);
    double _exp58r = 0, _exp58i = 0;
    c_exp2(_mul57r, _mul57i, &_exp58r, &_exp58i);
    double _abs59r = 0, _abs59i = 0;
    _abs59r = c_abs(x2r, x2i); _abs59i = 0;
    double _c60r = 0, _c60i = 0;
    _c60r = 1.0; _c60i = 0;
    double _add61r = 0, _add61i = 0;
    _add61r = _abs59r + _c60r; _add61i = _abs59i + _c60i;
    double _log62r = 0, _log62i = 0;
    c_log(_add61r, _add61i, &_log62r, &_log62i);
    double _mul63r = 0, _mul63i = 0;
    c_mul(_exp58r, _exp58i, _log62r, _log62i, &_mul63r, &_mul63i);
    double _add64r = 0, _add64i = 0;
    _add64r = x1r + x2r; _add64i = x1i + x2i;
    double _abs65r = 0, _abs65i = 0;
    _abs65r = c_abs(_add64r, _add64i); _abs65i = 0;
    double _add66r = 0, _add66i = 0;
    _add66r = _mul63r + _abs65r; _add66i = _mul63i + _abs65i;
    { int _idx = 19; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add66r; cIm[_idx] = _add66i; } }
    double _add67r = 0, _add67i = 0;
    _add67r = x1r + x2r; _add67i = x1i + x2i;
    double _sin68r = 0, _sin68i = 0;
    c_sin(_add67r, _add67i, &_sin68r, &_sin68i);
    double _sub69r = 0, _sub69i = 0;
    _sub69r = x1r - x2r; _sub69i = x1i - x2i;
    double _cos70r = 0, _cos70i = 0;
    c_cos(_sub69r, _sub69i, &_cos70r, &_cos70i);
    double _mul71r = 0, _mul71i = 0;
    c_mul(_sin68r, _sin68i, _cos70r, _cos70i, &_mul71r, &_mul71i);
    double _c72r = 0, _c72i = 0;
    _c72r = 0.0; _c72i = 1.0;
    double _re73r = 0, _re73i = 0;
    _re73r = x1r; _re73i = 0;
    double _im74r = 0, _im74i = 0;
    _im74r = x2i; _im74i = 0;
    double _mul75r = 0, _mul75i = 0;
    c_mul(_re73r, _re73i, _im74r, _im74i, &_mul75r, &_mul75i);
    double _mul76r = 0, _mul76i = 0;
    c_mul(_c72r, _c72i, _mul75r, _mul75i, &_mul76r, &_mul76i);
    double _add77r = 0, _add77i = 0;
    _add77r = _mul71r + _mul76r; _add77i = _mul71i + _mul76i;
    { int _idx = 24; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add77r; cIm[_idx] = _add77i; } }
    double _re78r = 0, _re78i = 0;
    _re78r = x1r; _re78i = 0;
    double _im79r = 0, _im79i = 0;
    _im79r = x2i; _im79i = 0;
    double _prod80r = 0, _prod80i = 0;
    c_mul(_re78r, _re78i, _im79r, _im79i, &_prod80r, &_prod80i);
    double _add81r = 0, _add81i = 0;
    _add81r = x1r + x2r; _add81i = x1i + x2i;
    double _abs82r = 0, _abs82i = 0;
    _abs82r = c_abs(_add81r, _add81i); _abs82i = 0;
    double _prod83r = 0, _prod83i = 0;
    c_mul(_prod80r, _prod80i, _abs82r, _abs82i, &_prod83r, &_prod83i);
    double _re84r = 0, _re84i = 0;
    _re84r = x2r; _re84i = 0;
    double _im85r = 0, _im85i = 0;
    _im85r = x1i; _im85i = 0;
    double _sum86r = 0, _sum86i = 0;
    _sum86r = _re84r + _im85r; _sum86i = _re84i + _im85i;
    double _add87r = 0, _add87i = 0;
    _add87r = _prod83r + _sum86r; _add87i = _prod83i + _sum86i;
    { int _idx = 29; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add87r; cIm[_idx] = _add87i; } }
    double _conj88r = 0, _conj88i = 0;
    _conj88r = x1r; _conj88i = -(x1i);
    double _c89r = 0, _c89i = 0;
    _c89r = 2.0; _c89i = 0;
    double _pow90r = 0, _pow90i = 0;
    c_mul(_conj88r, _conj88i, _conj88r, _conj88i, &_pow90r, &_pow90i);
    double _conj91r = 0, _conj91i = 0;
    _conj91r = x2r; _conj91i = -(x2i);
    double _c92r = 0, _c92i = 0;
    _c92r = 3.0; _c92i = 0;
    double _pow93r = 0, _pow93i = 0;
    c_mul(_conj91r, _conj91i, _conj91r, _conj91i, &_pow93r, &_pow93i);
    c_mul(_pow93r, _pow93i, _conj91r, _conj91i, &_pow93r, &_pow93i);
    double _add94r = 0, _add94i = 0;
    _add94r = _pow90r + _pow93r; _add94i = _pow90i + _pow93i;
    double _mul95r = 0, _mul95i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul95r, &_mul95i);
    double _sub96r = 0, _sub96i = 0;
    _sub96r = _add94r - _mul95r; _sub96i = _add94i - _mul95i;
    { int _idx = 34; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub96r; cIm[_idx] = _sub96i; } }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_252_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 36; j++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 6.0; _c1i = 0;
        double _mod2r = 0, _mod2i = 0;
        _mod2r = fmod(j, _c1r); _mod2i = 0;
        double k_r = _mod2r, k_i = _mod2i;
        double _c3r = 0, _c3i = 0;
        _c3r = 6.0; _c3i = 0;
        double _fdiv4r = 0, _fdiv4i = 0;
        c_div(j, 0, _c3r, _c3i, &_fdiv4r, &_fdiv4i);
        _fdiv4r = floor(_fdiv4r); _fdiv4i = 0;
        double _c5r = 0, _c5i = 0;
        _c5r = 1.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = _fdiv4r + _c5r; _add6i = _fdiv4i + _c5i;
        double r_r = _add6r, r_i = _add6i;
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        if (k_r == _c7r) {
            double _abs8r = 0, _abs8i = 0;
            _abs8r = c_abs(x1r, x1i); _abs8i = 0;
            double _c9r = 0, _c9i = 0;
            _c9r = 1.0; _c9i = 0;
            double _add10r = 0, _add10i = 0;
            _add10r = _abs8r + _c9r; _add10i = _abs8i + _c9i;
            double _log11r = 0, _log11i = 0;
            c_log(_add10r, _add10i, &_log11r, &_log11i);
            double _ang12r = 0, _ang12i = 0;
            _ang12r = c_arg(x2r, x2i); _ang12i = 0;
            double _sin13r = 0, _sin13i = 0;
            c_sin(_ang12r, _ang12i, &_sin13r, &_sin13i);
            double _add14r = 0, _add14i = 0;
            _add14r = _log11r + _sin13r; _add14i = _log11i + _sin13i;
            double _pow15r = 0, _pow15i = 0;
            c_powc(x1r, x1i, r_r, r_i, &_pow15r, &_pow15i);
            double _mul16r = 0, _mul16i = 0;
            c_mul(_add14r, _add14i, _pow15r, _pow15i, &_mul16r, &_mul16i);
            { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul16r; cIm[_idx] = _mul16i; } }
        } else {
            double _c17r = 0, _c17i = 0;
            _c17r = 2.0; _c17i = 0;
            if (k_r == _c17r) {
                double _ang18r = 0, _ang18i = 0;
                _ang18r = c_arg(x1r, x1i); _ang18i = 0;
                double _cos19r = 0, _cos19i = 0;
                c_cos(_ang18r, _ang18i, &_cos19r, &_cos19i);
                double _abs20r = 0, _abs20i = 0;
                _abs20r = c_abs(x2r, x2i); _abs20i = 0;
                double _sin21r = 0, _sin21i = 0;
                c_sin(_abs20r, _abs20i, &_sin21r, &_sin21i);
                double _sub22r = 0, _sub22i = 0;
                _sub22r = _cos19r - _sin21r; _sub22i = _cos19i - _sin21i;
                double _conj23r = 0, _conj23i = 0;
                _conj23r = x2r; _conj23i = -(x2i);
                double _pow24r = 0, _pow24i = 0;
                c_powc(_conj23r, _conj23i, r_r, r_i, &_pow24r, &_pow24i);
                double _mul25r = 0, _mul25i = 0;
                c_mul(_sub22r, _sub22i, _pow24r, _pow24i, &_mul25r, &_mul25i);
                { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul25r; cIm[_idx] = _mul25i; } }
            } else {
                double _c26r = 0, _c26i = 0;
                _c26r = 3.0; _c26i = 0;
                if (k_r == _c26r) {
                    double _re27r = 0, _re27i = 0;
                    _re27r = x1r; _re27i = 0;
                    double _im28r = 0, _im28i = 0;
                    _im28r = x2i; _im28i = 0;
                    double _mul29r = 0, _mul29i = 0;
                    c_mul(_re27r, _re27i, _im28r, _im28i, &_mul29r, &_mul29i);
                    double _re30r = 0, _re30i = 0;
                    _re30r = x2r; _re30i = 0;
                    double _im31r = 0, _im31i = 0;
                    _im31r = x1i; _im31i = 0;
                    double _mul32r = 0, _mul32i = 0;
                    c_mul(_re30r, _re30i, _im31r, _im31i, &_mul32r, &_mul32i);
                    double _add33r = 0, _add33i = 0;
                    _add33r = _mul29r + _mul32r; _add33i = _mul29i + _mul32i;
                    double _add34r = 0, _add34i = 0;
                    _add34r = x1r + x2r; _add34i = x1i + x2i;
                    double _pow35r = 0, _pow35i = 0;
                    c_powc(_add34r, _add34i, r_r, r_i, &_pow35r, &_pow35i);
                    double _mul36r = 0, _mul36i = 0;
                    c_mul(_add33r, _add33i, _pow35r, _pow35i, &_mul36r, &_mul36i);
                    { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul36r; cIm[_idx] = _mul36i; } }
                } else {
                    double _c37r = 0, _c37i = 0;
                    _c37r = 4.0; _c37i = 0;
                    if (k_r == _c37r) {
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
                        double _sub44r = 0, _sub44i = 0;
                        _sub44r = _pow40r - _pow43r; _sub44i = _pow40i - _pow43i;
                        double _c45r = 0, _c45i = 0;
                        _c45r = 0.0; _c45i = 1.0;
                        double _mul46r = 0, _mul46i = 0;
                        c_mul(x1r, x1i, x2r, x2i, &_mul46r, &_mul46i);
                        double _ang47r = 0, _ang47i = 0;
                        _ang47r = c_arg(_mul46r, _mul46i); _ang47i = 0;
                        double _mul48r = 0, _mul48i = 0;
                        c_mul(_c45r, _c45i, _ang47r, _ang47i, &_mul48r, &_mul48i);
                        double _exp49r = 0, _exp49i = 0;
                        c_exp2(_mul48r, _mul48i, &_exp49r, &_exp49i);
                        double _mul50r = 0, _mul50i = 0;
                        c_mul(_sub44r, _sub44i, _exp49r, _exp49i, &_mul50r, &_mul50i);
                        double _mul51r = 0, _mul51i = 0;
                        c_mul(_mul50r, _mul50i, r_r, r_i, &_mul51r, &_mul51i);
                        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul51r; cIm[_idx] = _mul51i; } }
                    } else {
                        double _c52r = 0, _c52i = 0;
                        _c52r = 5.0; _c52i = 0;
                        if (k_r == _c52r) {
                            double _mul53r = 0, _mul53i = 0;
                            c_mul(x1r, x1i, r_r, r_i, &_mul53r, &_mul53i);
                            double _sin54r = 0, _sin54i = 0;
                            c_sin(_mul53r, _mul53i, &_sin54r, &_sin54i);
                            double _div55r = 0, _div55i = 0;
                            c_div(x2r, x2i, r_r, r_i, &_div55r, &_div55i);
                            double _cos56r = 0, _cos56i = 0;
                            c_cos(_div55r, _div55i, &_cos56r, &_cos56i);
                            double _add57r = 0, _add57i = 0;
                            _add57r = _sin54r + _cos56r; _add57i = _sin54i + _cos56i;
                            double _sub58r = 0, _sub58i = 0;
                            _sub58r = x1r - x2r; _sub58i = x1i - x2i;
                            double _c59r = 0, _c59i = 0;
                            _c59r = 2.0; _c59i = 0;
                            double _pow60r = 0, _pow60i = 0;
                            c_mul(_sub58r, _sub58i, _sub58r, _sub58i, &_pow60r, &_pow60i);
                            double _mul61r = 0, _mul61i = 0;
                            c_mul(_add57r, _add57i, _pow60r, _pow60i, &_mul61r, &_mul61i);
                            { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul61r; cIm[_idx] = _mul61i; } }
                        } else {
                            double _mul62r = 0, _mul62i = 0;
                            c_mul(x1r, x1i, x2r, x2i, &_mul62r, &_mul62i);
                            double _abs63r = 0, _abs63i = 0;
                            _abs63r = c_abs(_mul62r, _mul62i); _abs63i = 0;
                            double _c64r = 0, _c64i = 0;
                            _c64r = 1.0; _c64i = 0;
                            double _add65r = 0, _add65i = 0;
                            _add65r = _abs63r + _c64r; _add65i = _abs63i + _c64i;
                            double _log66r = 0, _log66i = 0;
                            c_log(_add65r, _add65i, &_log66r, &_log66i);
                            double _add67r = 0, _add67i = 0;
                            _add67r = x1r + x2r; _add67i = x1i + x2i;
                            double _ang68r = 0, _ang68i = 0;
                            _ang68r = c_arg(_add67r, _add67i); _ang68i = 0;
                            double _add69r = 0, _add69i = 0;
                            _add69r = _log66r + _ang68r; _add69i = _log66i + _ang68i;
                            double _conj70r = 0, _conj70i = 0;
                            _conj70r = x2r; _conj70i = -(x2i);
                            double _add71r = 0, _add71i = 0;
                            _add71r = x1r + _conj70r; _add71i = x1i + _conj70i;
                            double _pow72r = 0, _pow72i = 0;
                            c_powc(_add71r, _add71i, r_r, r_i, &_pow72r, &_pow72i);
                            double _mul73r = 0, _mul73i = 0;
                            c_mul(_add69r, _add69i, _pow72r, _pow72i, &_mul73r, &_mul73i);
                            { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul73r; cIm[_idx] = _mul73i; } }
                        }
                    }
                }
            }
        }
    }
    double _c74r = 0, _c74i = 0;
    _c74r = 0.0; _c74i = 100.0;
    double _c75r = 0, _c75i = 0;
    _c75r = 4.0; _c75i = 0;
    double _pow76r = 0, _pow76i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow76r, &_pow76i);
    c_mul(_pow76r, _pow76i, _pow76r, _pow76i, &_pow76r, &_pow76i);
    double _mul77r = 0, _mul77i = 0;
    c_mul(_c74r, _c74i, _pow76r, _pow76i, &_mul77r, &_mul77i);
    double _c78r = 0, _c78i = 0;
    _c78r = 50.0; _c78i = 0;
    double _c79r = 0, _c79i = 0;
    _c79r = 2.0; _c79i = 0;
    double _pow80r = 0, _pow80i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow80r, &_pow80i);
    double _mul81r = 0, _mul81i = 0;
    c_mul(_c78r, _c78i, _pow80r, _pow80i, &_mul81r, &_mul81i);
    double _sub82r = 0, _sub82i = 0;
    _sub82r = _mul77r - _mul81r; _sub82i = _mul77i - _mul81i;
    double _c83r = 0, _c83i = 0;
    _c83r = 0.0; _c83i = 25.0;
    double _add84r = 0, _add84i = 0;
    _add84r = _sub82r + _c83r; _add84i = _sub82i + _c83i;
    { int _idx = 4; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add84r; cIm[_idx] = _add84i; } }
    double _c85r = 0, _c85i = 0;
    _c85r = 75.0; _c85i = 0;
    double _conj86r = 0, _conj86i = 0;
    _conj86r = x1r; _conj86i = -(x1i);
    double _mul87r = 0, _mul87i = 0;
    c_mul(_c85r, _c85i, _conj86r, _conj86i, &_mul87r, &_mul87i);
    double _c88r = 0, _c88i = 0;
    _c88r = 0.0; _c88i = 60.0;
    double _mul89r = 0, _mul89i = 0;
    c_mul(_c88r, _c88i, x2r, x2i, &_mul89r, &_mul89i);
    double _sub90r = 0, _sub90i = 0;
    _sub90r = _mul87r - _mul89r; _sub90i = _mul87i - _mul89i;
    double _c91r = 0, _c91i = 0;
    _c91r = 30.0; _c91i = 0;
    double _add92r = 0, _add92i = 0;
    _add92r = _sub90r + _c91r; _add92i = _sub90i + _c91i;
    { int _idx = 11; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add92r; cIm[_idx] = _add92i; } }
    double _c93r = 0, _c93i = 0;
    _c93r = 3.0; _c93i = 0;
    double _pow94r = 0, _pow94i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow94r, &_pow94i);
    c_mul(_pow94r, _pow94i, x1r, x1i, &_pow94r, &_pow94i);
    double _c95r = 0, _c95i = 0;
    _c95r = 3.0; _c95i = 0;
    double _pow96r = 0, _pow96i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow96r, &_pow96i);
    c_mul(_pow96r, _pow96i, x2r, x2i, &_pow96r, &_pow96i);
    double _add97r = 0, _add97i = 0;
    _add97r = _pow94r + _pow96r; _add97i = _pow94i + _pow96i;
    double _re98r = 0, _re98i = 0;
    _re98r = x1r; _re98i = 0;
    double _re99r = 0, _re99i = 0;
    _re99r = x2r; _re99i = 0;
    double _add100r = 0, _add100i = 0;
    _add100r = _re98r + _re99r; _add100i = _re98i + _re99i;
    double _c101r = 0, _c101i = 0;
    _c101r = 1.0; _c101i = 0;
    double _add102r = 0, _add102i = 0;
    _add102r = _add100r + _c101r; _add102i = _add100i + _c101i;
    double _div103r = 0, _div103i = 0;
    c_div(_add97r, _add97i, _add102r, _add102i, &_div103r, &_div103i);
    { int _idx = 18; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _div103r; cIm[_idx] = _div103i; } }
    double _add104r = 0, _add104i = 0;
    _add104r = x1r + x2r; _add104i = x1i + x2i;
    double _sin105r = 0, _sin105i = 0;
    c_sin(_add104r, _add104i, &_sin105r, &_sin105i);
    double _sub106r = 0, _sub106i = 0;
    _sub106r = x1r - x2r; _sub106i = x1i - x2i;
    double _cos107r = 0, _cos107i = 0;
    c_cos(_sub106r, _sub106i, &_cos107r, &_cos107i);
    double _mul108r = 0, _mul108i = 0;
    c_mul(_sin105r, _sin105i, _cos107r, _cos107i, &_mul108r, &_mul108i);
    double _c109r = 0, _c109i = 0;
    _c109r = 0.0; _c109i = 1.0;
    double _mul110r = 0, _mul110i = 0;
    c_mul(_mul108r, _mul108i, _c109r, _c109i, &_mul110r, &_mul110i);
    { int _idx = 25; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul110r; cIm[_idx] = _mul110i; } }
    double _add111r = 0, _add111i = 0;
    _add111r = x1r + x2r; _add111i = x1i + x2i;
    double _abs112r = 0, _abs112i = 0;
    _abs112r = c_abs(_add111r, _add111i); _abs112i = 0;
    double _c113r = 0, _c113i = 0;
    _c113r = 1.0; _c113i = 0;
    double _add114r = 0, _add114i = 0;
    _add114r = _abs112r + _c113r; _add114i = _abs112i + _c113i;
    double _log115r = 0, _log115i = 0;
    c_log(_add114r, _add114i, &_log115r, &_log115i);
    double _c116r = 0, _c116i = 0;
    _c116r = 2.0; _c116i = 0;
    double _pow117r = 0, _pow117i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow117r, &_pow117i);
    double _c118r = 0, _c118i = 0;
    _c118r = 2.0; _c118i = 0;
    double _pow119r = 0, _pow119i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow119r, &_pow119i);
    double _sub120r = 0, _sub120i = 0;
    _sub120r = _pow117r - _pow119r; _sub120i = _pow117i - _pow119i;
    double _mul121r = 0, _mul121i = 0;
    c_mul(_log115r, _log115i, _sub120r, _sub120i, &_mul121r, &_mul121i);
    { int _idx = 32; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul121r; cIm[_idx] = _mul121i; } }
    double _mul122r = 0, _mul122i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul122r, &_mul122i);
    double _re123r = 0, _re123i = 0;
    _re123r = _mul122r; _re123i = 0;
    double _sub124r = 0, _sub124i = 0;
    _sub124r = x1r - x2r; _sub124i = x1i - x2i;
    double _im125r = 0, _im125i = 0;
    _im125r = _sub124i; _im125i = 0;
    double _c126r = 0, _c126i = 0;
    _c126r = 0.0; _c126i = 1.0;
    double _mul127r = 0, _mul127i = 0;
    c_mul(_im125r, _im125i, _c126r, _c126i, &_mul127r, &_mul127i);
    double _add128r = 0, _add128i = 0;
    _add128r = _re123r + _mul127r; _add128i = _re123i + _mul127i;
    { int _idx = 34; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add128r; cIm[_idx] = _add128i; } }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_253_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 36; j++) {
        double angle = 0;
        double k = 0;
        double r = 0;
        double _c1r = 0, _c1i = 0;
        _c1r = 4.0; _c1i = 0;
        double _mod2r = 0, _mod2i = 0;
        _mod2r = fmod(j, _c1r); _mod2i = 0;
        double _c3r = 0, _c3i = 0;
        _c3r = 0.0; _c3i = 0;
        if (_mod2r == _c3r) {
            double _c4r = 0, _c4i = 0;
            _c4r = 4.0; _c4i = 0;
            double _fdiv5r = 0, _fdiv5i = 0;
            c_div(j, 0, _c4r, _c4i, &_fdiv5r, &_fdiv5i);
            _fdiv5r = floor(_fdiv5r); _fdiv5i = 0;
            k = _fdiv5r;
            double _add6r = 0, _add6i = 0;
            _add6r = x1r + x2r; _add6i = x1i + x2i;
            double _ang7r = 0, _ang7i = 0;
            _ang7r = c_arg(_add6r, _add6i); _ang7i = 0;
            double _mul8r = 0, _mul8i = 0;
            c_mul(_ang7r, _ang7i, k, 0, &_mul8r, &_mul8i);
            angle = _mul8r;
            double _re9r = 0, _re9i = 0;
            _re9r = x1r; _re9i = 0;
            double _pow10r = 0, _pow10i = 0;
            c_powr(_re9r, _re9i, k, &_pow10r, &_pow10i);
            double _im11r = 0, _im11i = 0;
            _im11r = x2i; _im11i = 0;
            double _pow12r = 0, _pow12i = 0;
            c_powr(_im11r, _im11i, k, &_pow12r, &_pow12i);
            double _add13r = 0, _add13i = 0;
            _add13r = _pow10r + _pow12r; _add13i = _pow10i + _pow12i;
            double _cos14r = 0, _cos14i = 0;
            c_cos(angle, 0, &_cos14r, &_cos14i);
            double _sin15r = 0, _sin15i = 0;
            c_sin(angle, 0, &_sin15r, &_sin15i);
            double _c16r = 0, _c16i = 0;
            _c16r = 0.0; _c16i = 1.0;
            double _mul17r = 0, _mul17i = 0;
            c_mul(_sin15r, _sin15i, _c16r, _c16i, &_mul17r, &_mul17i);
            double _add18r = 0, _add18i = 0;
            _add18r = _cos14r + _mul17r; _add18i = _cos14i + _mul17i;
            double _mul19r = 0, _mul19i = 0;
            c_mul(_add13r, _add13i, _add18r, _add18i, &_mul19r, &_mul19i);
            { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul19r; cIm[_idx] = _mul19i; } }
        } else {
            double _c20r = 0, _c20i = 0;
            _c20r = 5.0; _c20i = 0;
            double _mod21r = 0, _mod21i = 0;
            _mod21r = fmod(j, _c20r); _mod21i = 0;
            double _c22r = 0, _c22i = 0;
            _c22r = 0.0; _c22i = 0;
            if (_mod21r == _c22r) {
                double _c23r = 0, _c23i = 0;
                _c23r = 5.0; _c23i = 0;
                double _fdiv24r = 0, _fdiv24i = 0;
                c_div(j, 0, _c23r, _c23i, &_fdiv24r, &_fdiv24i);
                _fdiv24r = floor(_fdiv24r); _fdiv24i = 0;
                r = _fdiv24r;
                double _abs25r = 0, _abs25i = 0;
                _abs25r = c_abs(x1r, x1i); _abs25i = 0;
                double _mul26r = 0, _mul26i = 0;
                c_mul(_abs25r, _abs25i, r, 0, &_mul26r, &_mul26i);
                double _c27r = 0, _c27i = 0;
                _c27r = 1.0; _c27i = 0;
                double _add28r = 0, _add28i = 0;
                _add28r = _mul26r + _c27r; _add28i = _mul26i + _c27i;
                double _log29r = 0, _log29i = 0;
                c_log(_add28r, _add28i, &_log29r, &_log29i);
                double _conj30r = 0, _conj30i = 0;
                _conj30r = x2r; _conj30i = -(x2i);
                double _pow31r = 0, _pow31i = 0;
                c_powr(_conj30r, _conj30i, r, &_pow31r, &_pow31i);
                double _add32r = 0, _add32i = 0;
                _add32r = _log29r + _pow31r; _add32i = _log29i + _pow31i;
                { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add32r; cIm[_idx] = _add32i; } }
            } else {
                double _c33r = 0, _c33i = 0;
                _c33r = 3.0; _c33i = 0;
                double _mod34r = 0, _mod34i = 0;
                _mod34r = fmod(j, _c33r); _mod34i = 0;
                double _c35r = 0, _c35i = 0;
                _c35r = 1.0; _c35i = 0;
                if (_mod34r == _c35r) {
                    double _mul36r = 0, _mul36i = 0;
                    c_mul(x1r, x1i, j, 0, &_mul36r, &_mul36i);
                    double _sin37r = 0, _sin37i = 0;
                    c_sin(_mul36r, _mul36i, &_sin37r, &_sin37i);
                    double _mul38r = 0, _mul38i = 0;
                    c_mul(x2r, x2i, j, 0, &_mul38r, &_mul38i);
                    double _cos39r = 0, _cos39i = 0;
                    c_cos(_mul38r, _mul38i, &_cos39r, &_cos39i);
                    double _c40r = 0, _c40i = 0;
                    _c40r = 0.0; _c40i = 1.0;
                    double _mul41r = 0, _mul41i = 0;
                    c_mul(_cos39r, _cos39i, _c40r, _c40i, &_mul41r, &_mul41i);
                    double _add42r = 0, _add42i = 0;
                    _add42r = _sin37r + _mul41r; _add42i = _sin37i + _mul41i;
                    { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add42r; cIm[_idx] = _add42i; } }
                } else {
                    double _mul43r = 0, _mul43i = 0;
                    c_mul(x1r, x1i, x2r, x2i, &_mul43r, &_mul43i);
                    double _re44r = 0, _re44i = 0;
                    _re44r = _mul43r; _re44i = 0;
                    double _div45r = 0, _div45i = 0;
                    c_div(x1r, x1i, x2r, x2i, &_div45r, &_div45i);
                    double _im46r = 0, _im46i = 0;
                    _im46r = _div45i; _im46i = 0;
                    double _c47r = 0, _c47i = 0;
                    _c47r = 0.0; _c47i = 1.0;
                    double _mul48r = 0, _mul48i = 0;
                    c_mul(_im46r, _im46i, _c47r, _c47i, &_mul48r, &_mul48i);
                    double _add49r = 0, _add49i = 0;
                    _add49r = _re44r + _mul48r; _add49i = _re44i + _mul48i;
                    { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add49r; cIm[_idx] = _add49i; } }
                }
            }
        }
    }
    double _re50r = 0, _re50i = 0;
    _re50r = x1r; _re50i = 0;
    double _im51r = 0, _im51i = 0;
    _im51r = x2i; _im51i = 0;
    double _prod52r = 0, _prod52i = 0;
    c_mul(_re50r, _re50i, _im51r, _im51i, &_prod52r, &_prod52i);
    double _abs53r = 0, _abs53i = 0;
    _abs53r = c_abs(x1r, x1i); _abs53i = 0;
    double _abs54r = 0, _abs54i = 0;
    _abs54r = c_abs(x2r, x2i); _abs54i = 0;
    double _sum55r = 0, _sum55i = 0;
    _sum55r = _abs53r + _abs54r; _sum55i = _abs53i + _abs54i;
    double _c56r = 0, _c56i = 0;
    _c56r = 0.0; _c56i = 1.0;
    double _mul57r = 0, _mul57i = 0;
    c_mul(_sum55r, _sum55i, _c56r, _c56i, &_mul57r, &_mul57i);
    double _add58r = 0, _add58i = 0;
    _add58r = _prod52r + _mul57r; _add58i = _prod52i + _mul57i;
    { int _idx = 6; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add58r; cIm[_idx] = _add58i; } }
    double _c59r = 0, _c59i = 0;
    _c59r = 3.0; _c59i = 0;
    double _pow60r = 0, _pow60i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow60r, &_pow60i);
    c_mul(_pow60r, _pow60i, x1r, x1i, &_pow60r, &_pow60i);
    double _c61r = 0, _c61i = 0;
    _c61r = 2.0; _c61i = 0;
    double _pow62r = 0, _pow62i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow62r, &_pow62i);
    double _add63r = 0, _add63i = 0;
    _add63r = _pow60r + _pow62r; _add63i = _pow60i + _pow62i;
    double _c64r = 0, _c64i = 0;
    _c64r = 5.0; _c64i = 0;
    double _mul65r = 0, _mul65i = 0;
    c_mul(_c64r, _c64i, x1r, x1i, &_mul65r, &_mul65i);
    double _mul66r = 0, _mul66i = 0;
    c_mul(_mul65r, _mul65i, x2r, x2i, &_mul66r, &_mul66i);
    double _c67r = 0, _c67i = 0;
    _c67r = 0.0; _c67i = 1.0;
    double _mul68r = 0, _mul68i = 0;
    c_mul(_mul66r, _mul66i, _c67r, _c67i, &_mul68r, &_mul68i);
    double _sub69r = 0, _sub69i = 0;
    _sub69r = _add63r - _mul68r; _sub69i = _add63i - _mul68i;
    { int _idx = 13; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub69r; cIm[_idx] = _sub69i; } }
    double _add70r = 0, _add70i = 0;
    _add70r = x1r + x2r; _add70i = x1i + x2i;
    double _sin71r = 0, _sin71i = 0;
    c_sin(_add70r, _add70i, &_sin71r, &_sin71i);
    double _sub72r = 0, _sub72i = 0;
    _sub72r = x1r - x2r; _sub72i = x1i - x2i;
    double _cos73r = 0, _cos73i = 0;
    c_cos(_sub72r, _sub72i, &_cos73r, &_cos73i);
    double _c74r = 0, _c74i = 0;
    _c74r = 0.0; _c74i = 1.0;
    double _mul75r = 0, _mul75i = 0;
    c_mul(_cos73r, _cos73i, _c74r, _c74i, &_mul75r, &_mul75i);
    double _add76r = 0, _add76i = 0;
    _add76r = _sin71r + _mul75r; _add76i = _sin71i + _mul75i;
    { int _idx = 20; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add76r; cIm[_idx] = _add76i; } }
    double _abs77r = 0, _abs77i = 0;
    _abs77r = c_abs(x1r, x1i); _abs77i = 0;
    double _c78r = 0, _c78i = 0;
    _c78r = 1.0; _c78i = 0;
    double _add79r = 0, _add79i = 0;
    _add79r = _abs77r + _c78r; _add79i = _abs77i + _c78i;
    double _log80r = 0, _log80i = 0;
    c_log(_add79r, _add79i, &_log80r, &_log80i);
    double _conj81r = 0, _conj81i = 0;
    _conj81r = x2r; _conj81i = -(x2i);
    double _mul82r = 0, _mul82i = 0;
    c_mul(_log80r, _log80i, _conj81r, _conj81i, &_mul82r, &_mul82i);
    double _mul83r = 0, _mul83i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul83r, &_mul83i);
    double _sin84r = 0, _sin84i = 0;
    c_sin(_mul83r, _mul83i, &_sin84r, &_sin84i);
    double _sub85r = 0, _sub85i = 0;
    _sub85r = _mul82r - _sin84r; _sub85i = _mul82i - _sin84i;
    { int _idx = 27; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub85r; cIm[_idx] = _sub85i; } }
    double _re86r = 0, _re86i = 0;
    _re86r = x1r; _re86i = 0;
    double _c87r = 0, _c87i = 0;
    _c87r = 2.0; _c87i = 0;
    double _pow88r = 0, _pow88i = 0;
    c_mul(_re86r, _re86i, _re86r, _re86i, &_pow88r, &_pow88i);
    double _im89r = 0, _im89i = 0;
    _im89r = x2i; _im89i = 0;
    double _c90r = 0, _c90i = 0;
    _c90r = 2.0; _c90i = 0;
    double _pow91r = 0, _pow91i = 0;
    c_mul(_im89r, _im89i, _im89r, _im89i, &_pow91r, &_pow91i);
    double _sub92r = 0, _sub92i = 0;
    _sub92r = _pow88r - _pow91r; _sub92i = _pow88i - _pow91i;
    double _c93r = 0, _c93i = 0;
    _c93r = 2.0; _c93i = 0;
    double _re94r = 0, _re94i = 0;
    _re94r = x1r; _re94i = 0;
    double _mul95r = 0, _mul95i = 0;
    c_mul(_c93r, _c93i, _re94r, _re94i, &_mul95r, &_mul95i);
    double _im96r = 0, _im96i = 0;
    _im96r = x2i; _im96i = 0;
    double _mul97r = 0, _mul97i = 0;
    c_mul(_mul95r, _mul95i, _im96r, _im96i, &_mul97r, &_mul97i);
    double _c98r = 0, _c98i = 0;
    _c98r = 0.0; _c98i = 1.0;
    double _mul99r = 0, _mul99i = 0;
    c_mul(_mul97r, _mul97i, _c98r, _c98i, &_mul99r, &_mul99i);
    double _add100r = 0, _add100i = 0;
    _add100r = _sub92r + _mul99r; _add100i = _sub92i + _mul99i;
    { int _idx = 34; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add100r; cIm[_idx] = _add100i; } }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_254_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
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
        double k_r = _add4r, k_i = _add4i;
        double _c5r = 0, _c5i = 0;
        _c5r = 5.0; _c5i = 0;
        double _fdiv6r = 0, _fdiv6i = 0;
        c_div(j, 0, _c5r, _c5i, &_fdiv6r, &_fdiv6i);
        _fdiv6r = floor(_fdiv6r); _fdiv6i = 0;
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _fdiv6r + _c7r; _add8i = _fdiv6i + _c7i;
        double r_r = _add8r, r_i = _add8i;
        double _ang9r = 0, _ang9i = 0;
        _ang9r = c_arg(x1r, x1i); _ang9i = 0;
        double _sin10r = 0, _sin10i = 0;
        c_sin(j, 0, &_sin10r, &_sin10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(_ang9r, _ang9i, _sin10r, _sin10i, &_mul11r, &_mul11i);
        double _ang12r = 0, _ang12i = 0;
        _ang12r = c_arg(x2r, x2i); _ang12i = 0;
        double _cos13r = 0, _cos13i = 0;
        c_cos(j, 0, &_cos13r, &_cos13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_ang12r, _ang12i, _cos13r, _cos13i, &_mul14r, &_mul14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _mul11r + _mul14r; _add15i = _mul11i + _mul14i;
        double angle_r = _add15r, angle_i = _add15i;
        double _abs16r = 0, _abs16i = 0;
        _abs16r = c_abs(x1r, x1i); _abs16i = 0;
        double _pow17r = 0, _pow17i = 0;
        c_powc(_abs16r, _abs16i, k_r, k_i, &_pow17r, &_pow17i);
        double _abs18r = 0, _abs18i = 0;
        _abs18r = c_abs(x2r, x2i); _abs18i = 0;
        double _pow19r = 0, _pow19i = 0;
        c_powc(_abs18r, _abs18i, r_r, r_i, &_pow19r, &_pow19i);
        double _add20r = 0, _add20i = 0;
        _add20r = _pow17r + _pow19r; _add20i = _pow17i + _pow19i;
        double _abs21r = 0, _abs21i = 0;
        _abs21r = c_abs(x1r, x1i); _abs21i = 0;
        double _c22r = 0, _c22i = 0;
        _c22r = 1.0; _c22i = 0;
        double _add23r = 0, _add23i = 0;
        _add23r = _abs21r + _c22r; _add23i = _abs21i + _c22i;
        double _log24r = 0, _log24i = 0;
        c_log(_add23r, _add23i, &_log24r, &_log24i);
        double _abs25r = 0, _abs25i = 0;
        _abs25r = c_abs(x2r, x2i); _abs25i = 0;
        double _c26r = 0, _c26i = 0;
        _c26r = 1.0; _c26i = 0;
        double _add27r = 0, _add27i = 0;
        _add27r = _abs25r + _c26r; _add27i = _abs25i + _c26i;
        double _log28r = 0, _log28i = 0;
        c_log(_add27r, _add27i, &_log28r, &_log28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_log24r, _log24i, _log28r, _log28i, &_mul29r, &_mul29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _add20r + _mul29r; _add30i = _add20i + _mul29i;
        double magnitude_r = _add30r, magnitude_i = _add30i;
        double _cos31r = 0, _cos31i = 0;
        c_cos(angle_r, angle_i, &_cos31r, &_cos31i);
        double _sin32r = 0, _sin32i = 0;
        c_sin(angle_r, angle_i, &_sin32r, &_sin32i);
        double _c33r = 0, _c33i = 0;
        _c33r = 0.0; _c33i = 1.0;
        double _mul34r = 0, _mul34i = 0;
        c_mul(_sin32r, _sin32i, _c33r, _c33i, &_mul34r, &_mul34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _cos31r + _mul34r; _add35i = _cos31i + _mul34i;
        double _mul36r = 0, _mul36i = 0;
        c_mul(magnitude_r, magnitude_i, _add35r, _add35i, &_mul36r, &_mul36i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul36r; cIm[_idx] = _mul36i; } }
    }
    double _conj37r = 0, _conj37i = 0;
    _conj37r = x1r; _conj37i = -(x1i);
    double _c38r = 0, _c38i = 0;
    _c38r = 2.0; _c38i = 0;
    double _pow39r = 0, _pow39i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow39r, &_pow39i);
    double _mul40r = 0, _mul40i = 0;
    c_mul(_conj37r, _conj37i, _pow39r, _pow39i, &_mul40r, &_mul40i);
    double _c41r = 0, _c41i = 0;
    _c41r = 2.0; _c41i = 0;
    double _pow42r = 0, _pow42i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow42r, &_pow42i);
    double _conj43r = 0, _conj43i = 0;
    _conj43r = x2r; _conj43i = -(x2i);
    double _mul44r = 0, _mul44i = 0;
    c_mul(_pow42r, _pow42i, _conj43r, _conj43i, &_mul44r, &_mul44i);
    double _sub45r = 0, _sub45i = 0;
    _sub45r = _mul40r - _mul44r; _sub45i = _mul40i - _mul44i;
    { int _idx = 2; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub45r; cIm[_idx] = _sub45i; } }
    { int _idx = 7; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub45r; cIm[_idx] = _sub45i; } }
    { int _idx = 13; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub45r; cIm[_idx] = _sub45i; } }
    { int _idx = 21; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub45r; cIm[_idx] = _sub45i; } }
    { int _idx = 28; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub45r; cIm[_idx] = _sub45i; } }
    double _mul46r = 0, _mul46i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul46r, &_mul46i);
    double _sin47r = 0, _sin47i = 0;
    c_sin(_mul46r, _mul46i, &_sin47r, &_sin47i);
    double _add48r = 0, _add48i = 0;
    _add48r = x1r + x2r; _add48i = x1i + x2i;
    double _cos49r = 0, _cos49i = 0;
    c_cos(_add48r, _add48i, &_cos49r, &_cos49i);
    double _c50r = 0, _c50i = 0;
    _c50r = 0.0; _c50i = 1.0;
    double _mul51r = 0, _mul51i = 0;
    c_mul(_cos49r, _cos49i, _c50r, _c50i, &_mul51r, &_mul51i);
    double _add52r = 0, _add52i = 0;
    _add52r = _sin47r + _mul51r; _add52i = _sin47i + _mul51i;
    { int _idx = 4; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add52r; cIm[_idx] = _add52i; } }
    { int _idx = 10; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add52r; cIm[_idx] = _add52i; } }
    { int _idx = 18; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add52r; cIm[_idx] = _add52i; } }
    { int _idx = 26; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add52r; cIm[_idx] = _add52i; } }
    { int _idx = 34; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add52r; cIm[_idx] = _add52i; } }
    double _abs53r = 0, _abs53i = 0;
    _abs53r = c_abs(x1r, x1i); _abs53i = 0;
    double _abs54r = 0, _abs54i = 0;
    _abs54r = c_abs(x2r, x2i); _abs54i = 0;
    double _prod55r = 0, _prod55i = 0;
    c_mul(_abs53r, _abs53i, _abs54r, _abs54i, &_prod55r, &_prod55i);
    double _c56r = 0, _c56i = 0;
    _c56r = 0.0; _c56i = 1.0;
    double _ang57r = 0, _ang57i = 0;
    _ang57r = c_arg(x1r, x1i); _ang57i = 0;
    double _ang58r = 0, _ang58i = 0;
    _ang58r = c_arg(x2r, x2i); _ang58i = 0;
    double _sub59r = 0, _sub59i = 0;
    _sub59r = _ang57r - _ang58r; _sub59i = _ang57i - _ang58i;
    double _mul60r = 0, _mul60i = 0;
    c_mul(_c56r, _c56i, _sub59r, _sub59i, &_mul60r, &_mul60i);
    double _exp61r = 0, _exp61i = 0;
    c_exp2(_mul60r, _mul60i, &_exp61r, &_exp61i);
    double _mul62r = 0, _mul62i = 0;
    c_mul(_prod55r, _prod55i, _exp61r, _exp61i, &_mul62r, &_mul62i);
    { int _idx = 16; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul62r; cIm[_idx] = _mul62i; } }
    double _add63r = 0, _add63i = 0;
    _add63r = x1r + x2r; _add63i = x1i + x2i;
    double _abs64r = 0, _abs64i = 0;
    _abs64r = c_abs(_add63r, _add63i); _abs64i = 0;
    double _re65r = 0, _re65i = 0;
    _re65r = x1r; _re65i = 0;
    double _c66r = 0, _c66i = 0;
    _c66r = 2.0; _c66i = 0;
    double _pow67r = 0, _pow67i = 0;
    c_mul(_re65r, _re65i, _re65r, _re65i, &_pow67r, &_pow67i);
    double _sum68r = 0, _sum68i = 0;
    _sum68r = _abs64r + _pow67r; _sum68i = _abs64i + _pow67i;
    double _im69r = 0, _im69i = 0;
    _im69r = x2i; _im69i = 0;
    double _c70r = 0, _c70i = 0;
    _c70r = 2.0; _c70i = 0;
    double _pow71r = 0, _pow71i = 0;
    c_mul(_im69r, _im69i, _im69r, _im69i, &_pow71r, &_pow71i);
    double _sum72r = 0, _sum72i = 0;
    _sum72r = _sum68r + _pow71r; _sum72i = _sum68i + _pow71i;
    double _c73r = 0, _c73i = 0;
    _c73r = 1.0; _c73i = 0;
    double _c74r = 0, _c74i = 0;
    _c74r = 0.0; _c74i = 1.0;
    double _add75r = 0, _add75i = 0;
    _add75r = _c73r + _c74r; _add75i = _c73i + _c74i;
    double _mul76r = 0, _mul76i = 0;
    c_mul(_sum72r, _sum72i, _add75r, _add75i, &_mul76r, &_mul76i);
    { int _idx = 24; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul76r; cIm[_idx] = _mul76i; } }
    double _abs77r = 0, _abs77i = 0;
    _abs77r = c_abs(x1r, x1i); _abs77i = 0;
    double _c78r = 0, _c78i = 0;
    _c78r = 1.0; _c78i = 0;
    double _add79r = 0, _add79i = 0;
    _add79r = _abs77r + _c78r; _add79i = _abs77i + _c78i;
    double _log80r = 0, _log80i = 0;
    c_log(_add79r, _add79i, &_log80r, &_log80i);
    double _abs81r = 0, _abs81i = 0;
    _abs81r = c_abs(x2r, x2i); _abs81i = 0;
    double _c82r = 0, _c82i = 0;
    _c82r = 1.0; _c82i = 0;
    double _add83r = 0, _add83i = 0;
    _add83r = _abs81r + _c82r; _add83i = _abs81i + _c82i;
    double _log84r = 0, _log84i = 0;
    c_log(_add83r, _add83i, &_log84r, &_log84i);
    double _c85r = 0, _c85i = 0;
    _c85r = 0.0; _c85i = 1.0;
    double _mul86r = 0, _mul86i = 0;
    c_mul(_log84r, _log84i, _c85r, _c85i, &_mul86r, &_mul86i);
    double _add87r = 0, _add87i = 0;
    _add87r = _log80r + _mul86r; _add87i = _log80i + _mul86i;
    { int _idx = 34; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add87r; cIm[_idx] = _add87i; } }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_255: auto-stubbed (unhandled constructs in source) */
static void poly_255_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_256: auto-stubbed (unhandled constructs in source) */
static void poly_256_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_257_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
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
        double k_r = _add4r, k_i = _add4i;
        double _c5r = 0, _c5i = 0;
        _c5r = 2.0; _c5i = 0;
        double _pow6r = 0, _pow6i = 0;
        c_mul(j, 0, j, 0, &_pow6r, &_pow6i);
        double _re7r = 0, _re7i = 0;
        _re7r = x1r; _re7i = 0;
        double _mul8r = 0, _mul8i = 0;
        c_mul(_re7r, _re7i, j, 0, &_mul8r, &_mul8i);
        double _sin9r = 0, _sin9i = 0;
        c_sin(_mul8r, _mul8i, &_sin9r, &_sin9i);
        double _add10r = 0, _add10i = 0;
        _add10r = _pow6r + _sin9r; _add10i = _pow6i + _sin9i;
        double _im11r = 0, _im11i = 0;
        _im11r = x2i; _im11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(_im11r, _im11i, k_r, k_i, &_mul12r, &_mul12i);
        double _cos13r = 0, _cos13i = 0;
        c_cos(_mul12r, _mul12i, &_cos13r, &_cos13i);
        double _sub14r = 0, _sub14i = 0;
        _sub14r = _add10r - _cos13r; _sub14i = _add10i - _cos13i;
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
        c_div(_sub14r, _sub14i, _add20r, _add20i, &_div21r, &_div21i);
        double r_r = _div21r, r_i = _div21i;
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x1r, x1i); _ang22i = 0;
        double _pow23r = 0, _pow23i = 0;
        c_powc(_ang22r, _ang22i, k_r, k_i, &_pow23r, &_pow23i);
        double _ang24r = 0, _ang24i = 0;
        _ang24r = c_arg(x2r, x2i); _ang24i = 0;
        double _c25r = 0, _c25i = 0;
        _c25r = 3.0; _c25i = 0;
        double _mod26r = 0, _mod26i = 0;
        _mod26r = fmod(j, _c25r); _mod26i = 0;
        double _pow27r = 0, _pow27i = 0;
        c_powc(_ang24r, _ang24i, _mod26r, _mod26i, &_pow27r, &_pow27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _pow23r + _pow27r; _add28i = _pow23i + _pow27i;
        double angle_r = _add28r, angle_i = _add28i;
        double _re29r = 0, _re29i = 0;
        _re29r = x1r; _re29i = 0;
        double _pow30r = 0, _pow30i = 0;
        c_powc(_re29r, _re29i, k_r, k_i, &_pow30r, &_pow30i);
        double _im31r = 0, _im31i = 0;
        _im31r = x2i; _im31i = 0;
        double _pow32r = 0, _pow32i = 0;
        c_powr(_im31r, _im31i, j, &_pow32r, &_pow32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _pow30r + _pow32r; _add33i = _pow30i + _pow32i;
        double _mul34r = 0, _mul34i = 0;
        c_mul(r_r, r_i, _add33r, _add33i, &_mul34r, &_mul34i);
        double _c35r = 0, _c35i = 0;
        _c35r = 0.0; _c35i = 1.0;
        double _mul36r = 0, _mul36i = 0;
        c_mul(_c35r, _c35i, angle_r, angle_i, &_mul36r, &_mul36i);
        double _exp37r = 0, _exp37i = 0;
        c_exp2(_mul36r, _mul36i, &_exp37r, &_exp37i);
        double _mul38r = 0, _mul38i = 0;
        c_mul(_mul34r, _mul34i, _exp37r, _exp37i, &_mul38r, &_mul38i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul38r; cIm[_idx] = _mul38i; } }
    }
    for (int k = 1; k < 6; k++) {
        double _re39r = 0, _re39i = 0;
        _re39r = x1r; _re39i = 0;
        double _add40r = 0, _add40i = 0;
        _add40r = _re39r + k_r; _add40i = _re39i + k_i;
        double _im41r = 0, _im41i = 0;
        _im41r = x2i; _im41i = 0;
        double _mul42r = 0, _mul42i = 0;
        c_mul(_im41r, _im41i, k_r, k_i, &_mul42r, &_mul42i);
        double _sub43r = 0, _sub43i = 0;
        _sub43r = _add40r - _mul42r; _sub43i = _add40i - _mul42i;
        r_r = _sub43r; r_i = _sub43i;
        double _add44r = 0, _add44i = 0;
        _add44r = x1r + k_r; _add44i = x1i + k_i;
        double _ang45r = 0, _ang45i = 0;
        _ang45r = c_arg(_add44r, _add44i); _ang45i = 0;
        double _add46r = 0, _add46i = 0;
        _add46r = x2r + k_r; _add46i = x2i + k_i;
        double _ang47r = 0, _ang47i = 0;
        _ang47r = c_arg(_add46r, _add46i); _ang47i = 0;
        double _sub48r = 0, _sub48i = 0;
        _sub48r = _ang45r - _ang47r; _sub48i = _ang45i - _ang47i;
        angle_r = _sub48r; angle_i = _sub48i;
        double _c49r = 0, _c49i = 0;
        _c49r = 5.0; _c49i = 0;
        double _mul50r = 0, _mul50i = 0;
        c_mul(_c49r, _c49i, k_r, k_i, &_mul50r, &_mul50i);
        double index_r = _mul50r, index_i = _mul50i;
        double _c51r = 0, _c51i = 0;
        _c51r = 35.0; _c51i = 0;
        if (index_r <= _c51r) {
            double _c52r = 0, _c52i = 0;
            _c52r = 0.0; _c52i = 1.0;
            double _mul53r = 0, _mul53i = 0;
            c_mul(_c52r, _c52i, angle_r, angle_i, &_mul53r, &_mul53i);
            double _exp54r = 0, _exp54i = 0;
            c_exp2(_mul53r, _mul53i, &_exp54r, &_exp54i);
            double _mul55r = 0, _mul55i = 0;
            c_mul(r_r, r_i, _exp54r, _exp54i, &_mul55r, &_mul55i);
            double _mul56r = 0, _mul56i = 0;
            c_mul(x1r, x1i, k_r, k_i, &_mul56r, &_mul56i);
            double _sin57r = 0, _sin57i = 0;
            c_sin(_mul56r, _mul56i, &_sin57r, &_sin57i);
            double _mul58r = 0, _mul58i = 0;
            c_mul(x2r, x2i, k_r, k_i, &_mul58r, &_mul58i);
            double _cos59r = 0, _cos59i = 0;
            c_cos(_mul58r, _mul58i, &_cos59r, &_cos59i);
            double _add60r = 0, _add60i = 0;
            _add60r = _sin57r + _cos59r; _add60i = _sin57i + _cos59i;
            double _mul61r = 0, _mul61i = 0;
            c_mul(_mul55r, _mul55i, _add60r, _add60i, &_mul61r, &_mul61i);
            { int _idx = ((int)(index_r) - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul61r; cIm[_idx] = _mul61i; } }
        }
    }
    double _conj62r = 0, _conj62i = 0;
    _conj62r = x1r; _conj62i = -(x1i);
    double _c63r = 0, _c63i = 0;
    _c63r = 2.0; _c63i = 0;
    double _pow64r = 0, _pow64i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow64r, &_pow64i);
    double _mul65r = 0, _mul65i = 0;
    c_mul(_conj62r, _conj62i, _pow64r, _pow64i, &_mul65r, &_mul65i);
    double _add66r = 0, _add66i = 0;
    _add66r = x1r + x2r; _add66i = x1i + x2i;
    double _sin67r = 0, _sin67i = 0;
    c_sin(_add66r, _add66i, &_sin67r, &_sin67i);
    double _add68r = 0, _add68i = 0;
    _add68r = _mul65r + _sin67r; _add68i = _mul65i + _sin67i;
    { int _idx = 6; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add68r; cIm[_idx] = _add68i; } }
    double _abs69r = 0, _abs69i = 0;
    _abs69r = c_abs(x1r, x1i); _abs69i = 0;
    double _c70r = 0, _c70i = 0;
    _c70r = 1.0; _c70i = 0;
    double _add71r = 0, _add71i = 0;
    _add71r = _abs69r + _c70r; _add71i = _abs69i + _c70i;
    double _log72r = 0, _log72i = 0;
    c_log(_add71r, _add71i, &_log72r, &_log72i);
    double _cos73r = 0, _cos73i = 0;
    c_cos(x2r, x2i, &_cos73r, &_cos73i);
    double _mul74r = 0, _mul74i = 0;
    c_mul(_log72r, _log72i, _cos73r, _cos73i, &_mul74r, &_mul74i);
    double _c75r = 0, _c75i = 0;
    _c75r = 0.0; _c75i = 1.0;
    double _mul76r = 0, _mul76i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul76r, &_mul76i);
    double _sin77r = 0, _sin77i = 0;
    c_sin(_mul76r, _mul76i, &_sin77r, &_sin77i);
    double _mul78r = 0, _mul78i = 0;
    c_mul(_c75r, _c75i, _sin77r, _sin77i, &_mul78r, &_mul78i);
    double _sub79r = 0, _sub79i = 0;
    _sub79r = _mul74r - _mul78r; _sub79i = _mul74i - _mul78i;
    { int _idx = 13; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub79r; cIm[_idx] = _sub79i; } }
    double _abs80r = 0, _abs80i = 0;
    _abs80r = c_abs(x1r, x1i); _abs80i = 0;
    double _c81r = 0, _c81i = 0;
    _c81r = 3.0; _c81i = 0;
    double _pow82r = 0, _pow82i = 0;
    c_mul(_abs80r, _abs80i, _abs80r, _abs80i, &_pow82r, &_pow82i);
    c_mul(_pow82r, _pow82i, _abs80r, _abs80i, &_pow82r, &_pow82i);
    double _abs83r = 0, _abs83i = 0;
    _abs83r = c_abs(x2r, x2i); _abs83i = 0;
    double _c84r = 0, _c84i = 0;
    _c84r = 2.0; _c84i = 0;
    double _pow85r = 0, _pow85i = 0;
    c_mul(_abs83r, _abs83i, _abs83r, _abs83i, &_pow85r, &_pow85i);
    double _sub86r = 0, _sub86i = 0;
    _sub86r = _pow82r - _pow85r; _sub86i = _pow82i - _pow85i;
    double _c87r = 0, _c87i = 0;
    _c87r = 0.0; _c87i = 1.0;
    double _mul88r = 0, _mul88i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul88r, &_mul88i);
    double _ang89r = 0, _ang89i = 0;
    _ang89r = c_arg(_mul88r, _mul88i); _ang89i = 0;
    double _mul90r = 0, _mul90i = 0;
    c_mul(_c87r, _c87i, _ang89r, _ang89i, &_mul90r, &_mul90i);
    double _add91r = 0, _add91i = 0;
    _add91r = _sub86r + _mul90r; _add91i = _sub86i + _mul90i;
    { int _idx = 20; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add91r; cIm[_idx] = _add91i; } }
    double _c92r = 0, _c92i = 0;
    _c92r = 2.0; _c92i = 0;
    double _pow93r = 0, _pow93i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow93r, &_pow93i);
    double _re94r = 0, _re94i = 0;
    _re94r = _pow93r; _re94i = 0;
    double _c95r = 0, _c95i = 0;
    _c95r = 3.0; _c95i = 0;
    double _pow96r = 0, _pow96i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow96r, &_pow96i);
    c_mul(_pow96r, _pow96i, x2r, x2i, &_pow96r, &_pow96i);
    double _im97r = 0, _im97i = 0;
    _im97r = _pow96i; _im97i = 0;
    double _add98r = 0, _add98i = 0;
    _add98r = _re94r + _im97r; _add98i = _re94i + _im97i;
    double _c99r = 0, _c99i = 0;
    _c99r = 0.0; _c99i = 2.0;
    double _mul100r = 0, _mul100i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul100r, &_mul100i);
    double _re101r = 0, _re101i = 0;
    _re101r = _mul100r; _re101i = 0;
    double _mul102r = 0, _mul102i = 0;
    c_mul(_c99r, _c99i, _re101r, _re101i, &_mul102r, &_mul102i);
    double _sub103r = 0, _sub103i = 0;
    _sub103r = _add98r - _mul102r; _sub103i = _add98i - _mul102i;
    { int _idx = 27; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub103r; cIm[_idx] = _sub103i; } }
    double _re104r = 0, _re104i = 0;
    _re104r = x1r; _re104i = 0;
    double _re105r = 0, _re105i = 0;
    _re105r = x2r; _re105i = 0;
    double _prod106r = 0, _prod106i = 0;
    c_mul(_re104r, _re104i, _re105r, _re105i, &_prod106r, &_prod106i);
    double _im107r = 0, _im107i = 0;
    _im107r = x1i; _im107i = 0;
    double _im108r = 0, _im108i = 0;
    _im108r = x2i; _im108i = 0;
    double _sum109r = 0, _sum109i = 0;
    _sum109r = _im107r + _im108r; _sum109i = _im107i + _im108i;
    double _c110r = 0, _c110i = 0;
    _c110r = 0.0; _c110i = 1.0;
    double _mul111r = 0, _mul111i = 0;
    c_mul(_sum109r, _sum109i, _c110r, _c110i, &_mul111r, &_mul111i);
    double _add112r = 0, _add112i = 0;
    _add112r = _prod106r + _mul111r; _add112i = _prod106i + _mul111i;
    { int _idx = 34; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add112r; cIm[_idx] = _add112i; } }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_258_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 36; j++) {
        double _ang1r = 0, _ang1i = 0;
        _ang1r = c_arg(x1r, x1i); _ang1i = 0;
        double _mul2r = 0, _mul2i = 0;
        c_mul(j, 0, _ang1r, _ang1i, &_mul2r, &_mul2i);
        double _ang3r = 0, _ang3i = 0;
        _ang3r = c_arg(x2r, x2i); _ang3i = 0;
        double _mul4r = 0, _mul4i = 0;
        c_mul(j, 0, _ang3r, _ang3i, &_mul4r, &_mul4i);
        double _cos5r = 0, _cos5i = 0;
        c_cos(_mul4r, _mul4i, &_cos5r, &_cos5i);
        double _add6r = 0, _add6i = 0;
        _add6r = _mul2r + _cos5r; _add6i = _mul2i + _cos5i;
        double _sin7r = 0, _sin7i = 0;
        c_sin(_add6r, _add6i, &_sin7r, &_sin7i);
        double _re8r = 0, _re8i = 0;
        _re8r = x1r; _re8i = 0;
        double _im9r = 0, _im9i = 0;
        _im9r = x2i; _im9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(_re8r, _re8i, _im9r, _im9i, &_mul10r, &_mul10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _sin7r + _mul10r; _add11i = _sin7i + _mul10i;
        double angle_r = _add11r, angle_i = _add11i;
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
        double _re17r = 0, _re17i = 0;
        _re17r = x1r; _re17i = 0;
        double _c18r = 0, _c18i = 0;
        _c18r = 4.0; _c18i = 0;
        double _mod19r = 0, _mod19i = 0;
        _mod19r = fmod(j, _c18r); _mod19i = 0;
        double _c20r = 0, _c20i = 0;
        _c20r = 1.0; _c20i = 0;
        double _add21r = 0, _add21i = 0;
        _add21r = _mod19r + _c20r; _add21i = _mod19i + _c20i;
        double _pow22r = 0, _pow22i = 0;
        c_powc(_re17r, _re17i, _add21r, _add21i, &_pow22r, &_pow22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _log16r + _pow22r; _add23i = _log16i + _pow22i;
        double _im24r = 0, _im24i = 0;
        _im24r = x2i; _im24i = 0;
        double _c25r = 0, _c25i = 0;
        _c25r = 3.0; _c25i = 0;
        double _mod26r = 0, _mod26i = 0;
        _mod26r = fmod(j, _c25r); _mod26i = 0;
        double _c27r = 0, _c27i = 0;
        _c27r = 1.0; _c27i = 0;
        double _add28r = 0, _add28i = 0;
        _add28r = _mod26r + _c27r; _add28i = _mod26i + _c27i;
        double _pow29r = 0, _pow29i = 0;
        c_powc(_im24r, _im24i, _add28r, _add28i, &_pow29r, &_pow29i);
        double _sub30r = 0, _sub30i = 0;
        _sub30r = _add23r - _pow29r; _sub30i = _add23i - _pow29i;
        double _re31r = 0, _re31i = 0;
        _re31r = x1r; _re31i = 0;
        double _im32r = 0, _im32i = 0;
        _im32r = x2i; _im32i = 0;
        double _prod33r = 0, _prod33i = 0;
        c_mul(_re31r, _re31i, _im32r, _im32i, &_prod33r, &_prod33i);
        double _prod34r = 0, _prod34i = 0;
        c_mul(_prod33r, _prod33i, j, 0, &_prod34r, &_prod34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _sub30r + _prod34r; _add35i = _sub30i + _prod34i;
        double magnitude_r = _add35r, magnitude_i = _add35i;
        double _cos36r = 0, _cos36i = 0;
        c_cos(angle_r, angle_i, &_cos36r, &_cos36i);
        double _c37r = 0, _c37i = 0;
        _c37r = 0.0; _c37i = 1.0;
        double _sin38r = 0, _sin38i = 0;
        c_sin(angle_r, angle_i, &_sin38r, &_sin38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(_c37r, _c37i, _sin38r, _sin38i, &_mul39r, &_mul39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _cos36r + _mul39r; _add40i = _cos36i + _mul39i;
        double _mul41r = 0, _mul41i = 0;
        c_mul(magnitude_r, magnitude_i, _add40r, _add40i, &_mul41r, &_mul41i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul41r; cIm[_idx] = _mul41i; } }
    }
    double _conj42r = 0, _conj42i = 0;
    _conj42r = x1r; _conj42i = -(x1i);
    double _c43r = 0, _c43i = 0;
    _c43r = 2.0; _c43i = 0;
    double _pow44r = 0, _pow44i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow44r, &_pow44i);
    double _mul45r = 0, _mul45i = 0;
    c_mul(_conj42r, _conj42i, _pow44r, _pow44i, &_mul45r, &_mul45i);
    double _mul46r = 0, _mul46i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul46r, &_mul46i);
    double _sin47r = 0, _sin47i = 0;
    c_sin(_mul46r, _mul46i, &_sin47r, &_sin47i);
    double _sub48r = 0, _sub48i = 0;
    _sub48r = x1r - x2r; _sub48i = x1i - x2i;
    double _cos49r = 0, _cos49i = 0;
    c_cos(_sub48r, _sub48i, &_cos49r, &_cos49i);
    double _mul50r = 0, _mul50i = 0;
    c_mul(_sin47r, _sin47i, _cos49r, _cos49i, &_mul50r, &_mul50i);
    double _add51r = 0, _add51i = 0;
    _add51r = _mul45r + _mul50r; _add51i = _mul45i + _mul50i;
    { int _idx = 2; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add51r; cIm[_idx] = _add51i; } }
    double _c52r = 0, _c52i = 0;
    _c52r = 2.0; _c52i = 0;
    double _pow53r = 0, _pow53i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow53r, &_pow53i);
    double _c54r = 0, _c54i = 0;
    _c54r = 2.0; _c54i = 0;
    double _pow55r = 0, _pow55i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow55r, &_pow55i);
    double _add56r = 0, _add56i = 0;
    _add56r = _pow53r + _pow55r; _add56i = _pow53i + _pow55i;
    double _re57r = 0, _re57i = 0;
    _re57r = _add56r; _re57i = 0;
    double _c58r = 0, _c58i = 0;
    _c58r = 0.0; _c58i = 1.0;
    double _mul59r = 0, _mul59i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul59r, &_mul59i);
    double _im60r = 0, _im60i = 0;
    _im60r = _mul59i; _im60i = 0;
    double _mul61r = 0, _mul61i = 0;
    c_mul(_c58r, _c58i, _im60r, _im60i, &_mul61r, &_mul61i);
    double _add62r = 0, _add62i = 0;
    _add62r = _re57r + _mul61r; _add62i = _re57i + _mul61i;
    { int _idx = 7; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add62r; cIm[_idx] = _add62i; } }
    double _add63r = 0, _add63i = 0;
    _add63r = x1r + x2r; _add63i = x1i + x2i;
    double _abs64r = 0, _abs64i = 0;
    _abs64r = c_abs(_add63r, _add63i); _abs64i = 0;
    double _c65r = 0, _c65i = 0;
    _c65r = 1.0; _c65i = 0;
    double _add66r = 0, _add66i = 0;
    _add66r = _abs64r + _c65r; _add66i = _abs64i + _c65i;
    double _log67r = 0, _log67i = 0;
    c_log(_add66r, _add66i, &_log67r, &_log67i);
    double _c68r = 0, _c68i = 0;
    _c68r = 0.0; _c68i = 1.0;
    double _sub69r = 0, _sub69i = 0;
    _sub69r = x1r - x2r; _sub69i = x1i - x2i;
    double _ang70r = 0, _ang70i = 0;
    _ang70r = c_arg(_sub69r, _sub69i); _ang70i = 0;
    double _mul71r = 0, _mul71i = 0;
    c_mul(_c68r, _c68i, _ang70r, _ang70i, &_mul71r, &_mul71i);
    double _add72r = 0, _add72i = 0;
    _add72r = _log67r + _mul71r; _add72i = _log67i + _mul71i;
    { int _idx = 14; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add72r; cIm[_idx] = _add72i; } }
    double _sin73r = 0, _sin73i = 0;
    c_sin(x1r, x1i, &_sin73r, &_sin73i);
    double _c74r = 0, _c74i = 0;
    _c74r = 3.0; _c74i = 0;
    double _pow75r = 0, _pow75i = 0;
    c_mul(_sin73r, _sin73i, _sin73r, _sin73i, &_pow75r, &_pow75i);
    c_mul(_pow75r, _pow75i, _sin73r, _sin73i, &_pow75r, &_pow75i);
    double _cos76r = 0, _cos76i = 0;
    c_cos(x2r, x2i, &_cos76r, &_cos76i);
    double _c77r = 0, _c77i = 0;
    _c77r = 3.0; _c77i = 0;
    double _pow78r = 0, _pow78i = 0;
    c_mul(_cos76r, _cos76i, _cos76r, _cos76i, &_pow78r, &_pow78i);
    c_mul(_pow78r, _pow78i, _cos76r, _cos76i, &_pow78r, &_pow78i);
    double _sub79r = 0, _sub79i = 0;
    _sub79r = _pow75r - _pow78r; _sub79i = _pow75i - _pow78i;
    double _c80r = 0, _c80i = 0;
    _c80r = 0.0; _c80i = 1.0;
    double _sin81r = 0, _sin81i = 0;
    c_sin(x1r, x1i, &_sin81r, &_sin81i);
    double _cos82r = 0, _cos82i = 0;
    c_cos(x2r, x2i, &_cos82r, &_cos82i);
    double _mul83r = 0, _mul83i = 0;
    c_mul(_sin81r, _sin81i, _cos82r, _cos82i, &_mul83r, &_mul83i);
    double _mul84r = 0, _mul84i = 0;
    c_mul(_c80r, _c80i, _mul83r, _mul83i, &_mul84r, &_mul84i);
    double _add85r = 0, _add85i = 0;
    _add85r = _sub79r + _mul84r; _add85i = _sub79i + _mul84i;
    { int _idx = 21; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add85r; cIm[_idx] = _add85i; } }
    double _mul86r = 0, _mul86i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul86r, &_mul86i);
    double _re87r = 0, _re87i = 0;
    _re87r = _mul86r; _re87i = 0;
    double _add88r = 0, _add88i = 0;
    _add88r = x1r + x2r; _add88i = x1i + x2i;
    double _im89r = 0, _im89i = 0;
    _im89r = _add88i; _im89i = 0;
    double _c90r = 0, _c90i = 0;
    _c90r = 0.0; _c90i = 1.0;
    double _mul91r = 0, _mul91i = 0;
    c_mul(_im89r, _im89i, _c90r, _c90i, &_mul91r, &_mul91i);
    double _add92r = 0, _add92i = 0;
    _add92r = _re87r + _mul91r; _add92i = _re87i + _mul91i;
    { int _idx = 28; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add92r; cIm[_idx] = _add92i; } }
    double _abs93r = 0, _abs93i = 0;
    _abs93r = c_abs(x1r, x1i); _abs93i = 0;
    double _abs94r = 0, _abs94i = 0;
    _abs94r = c_abs(x2r, x2i); _abs94i = 0;
    double _prod95r = 0, _prod95i = 0;
    c_mul(_abs93r, _abs93i, _abs94r, _abs94i, &_prod95r, &_prod95i);
    double _prod96r = 0, _prod96i = 0;
    c_mul(_prod95r, _prod95i, j, 0, &_prod96r, &_prod96i);
    double _c97r = 0, _c97i = 0;
    _c97r = 0.0; _c97i = 1.0;
    double _re98r = 0, _re98i = 0;
    _re98r = x1r; _re98i = 0;
    double _im99r = 0, _im99i = 0;
    _im99r = x2i; _im99i = 0;
    double _sum100r = 0, _sum100i = 0;
    _sum100r = _re98r + _im99r; _sum100i = _re98i + _im99i;
    double _mul101r = 0, _mul101i = 0;
    c_mul(_c97r, _c97i, _sum100r, _sum100i, &_mul101r, &_mul101i);
    double _add102r = 0, _add102i = 0;
    _add102r = _prod96r + _mul101r; _add102i = _prod96i + _mul101i;
    { int _idx = 34; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add102r; cIm[_idx] = _add102i; } }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_259_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 36; j++) {
        double _re1r = 0, _re1i = 0;
        _re1r = x1r; _re1i = 0;
        double _pow2r = 0, _pow2i = 0;
        c_powr(_re1r, _re1i, j, &_pow2r, &_pow2i);
        double _im3r = 0, _im3i = 0;
        _im3r = x2i; _im3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 35.0; _c4i = 0;
        double _sub5r = 0, _sub5i = 0;
        _sub5r = _c4r - j; _sub5i = _c4i - 0;
        double _pow6r = 0, _pow6i = 0;
        c_powc(_im3r, _im3i, _sub5r, _sub5i, &_pow6r, &_pow6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(_pow2r, _pow2i, _pow6r, _pow6i, &_mul7r, &_mul7i);
        double _conj8r = 0, _conj8i = 0;
        _conj8r = x1r; _conj8i = -(x1i);
        double _ang9r = 0, _ang9i = 0;
        _ang9r = c_arg(x2r, x2i); _ang9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(j, 0, _ang9r, _ang9i, &_mul10r, &_mul10i);
        double _sin11r = 0, _sin11i = 0;
        c_sin(_mul10r, _mul10i, &_sin11r, &_sin11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_conj8r, _conj8i, _sin11r, _sin11i, &_mul12r, &_mul12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _mul7r + _mul12r; _add13i = _mul7i + _mul12i;
        double _abs14r = 0, _abs14i = 0;
        _abs14r = c_abs(x1r, x1i); _abs14i = 0;
        double _abs15r = 0, _abs15i = 0;
        _abs15r = c_abs(x2r, x2i); _abs15i = 0;
        double _add16r = 0, _add16i = 0;
        _add16r = _abs14r + _abs15r; _add16i = _abs14i + _abs15i;
        double _c17r = 0, _c17i = 0;
        _c17r = 1.0; _c17i = 0;
        double _add18r = 0, _add18i = 0;
        _add18r = _add16r + _c17r; _add18i = _add16i + _c17i;
        double _log19r = 0, _log19i = 0;
        c_log(_add18r, _add18i, &_log19r, &_log19i);
        double _add20r = 0, _add20i = 0;
        _add20r = x1r + x2r; _add20i = x1i + x2i;
        double _ang21r = 0, _ang21i = 0;
        _ang21r = c_arg(_add20r, _add20i); _ang21i = 0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(j, 0, _ang21r, _ang21i, &_mul22r, &_mul22i);
        double _cos23r = 0, _cos23i = 0;
        c_cos(_mul22r, _mul22i, &_cos23r, &_cos23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_log19r, _log19i, _cos23r, _cos23i, &_mul24r, &_mul24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _add13r + _mul24r; _add25i = _add13i + _mul24i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add25r; cIm[_idx] = _add25i; } }
    }
    for (int k = 1; k < 6; k++) {
        double _c26r = 0, _c26i = 0;
        _c26r = 5.0; _c26i = 0;
        double _add27r = 0, _add27i = 0;
        _add27r = k + _c26r; _add27i = 0 + _c26i;
        double r_r = _add27r, r_i = _add27i;
        double _abs28r = 0, _abs28i = 0;
        _abs28r = c_abs(x1r, x1i); _abs28i = 0;
        double _pow29r = 0, _pow29i = 0;
        c_powr(_abs28r, _abs28i, k, &_pow29r, &_pow29i);
        double _abs30r = 0, _abs30i = 0;
        _abs30r = c_abs(x2r, x2i); _abs30i = 0;
        double _c31r = 0, _c31i = 0;
        _c31r = 5.0; _c31i = 0;
        double _sub32r = 0, _sub32i = 0;
        _sub32r = _c31r - k; _sub32i = _c31i - 0;
        double _pow33r = 0, _pow33i = 0;
        c_powc(_abs30r, _abs30i, _sub32r, _sub32i, &_pow33r, &_pow33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(_pow29r, _pow29i, _pow33r, _pow33i, &_mul34r, &_mul34i);
        double _c35r = 0, _c35i = 0;
        _c35r = 0.0; _c35i = 1.0;
        double _ang36r = 0, _ang36i = 0;
        _ang36r = c_arg(x1r, x1i); _ang36i = 0;
        double _ang37r = 0, _ang37i = 0;
        _ang37r = c_arg(x2r, x2i); _ang37i = 0;
        double _sub38r = 0, _sub38i = 0;
        _sub38r = _ang36r - _ang37r; _sub38i = _ang36i - _ang37i;
        double _mul39r = 0, _mul39i = 0;
        c_mul(_c35r, _c35i, _sub38r, _sub38i, &_mul39r, &_mul39i);
        double _exp40r = 0, _exp40i = 0;
        c_exp2(_mul39r, _mul39i, &_exp40r, &_exp40i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(_mul34r, _mul34i, _exp40r, _exp40i, &_mul41r, &_mul41i);
        cRe[((int)(r_r) - 1)] += _mul41r; cIm[((int)(r_r) - 1)] += _mul41i;
    }
    for (int m = 6; m < 11; m++) {
        double _re42r = 0, _re42i = 0;
        _re42r = x1r; _re42i = 0;
        double _mul43r = 0, _mul43i = 0;
        c_mul(_re42r, _re42i, m, 0, &_mul43r, &_mul43i);
        double _sin44r = 0, _sin44i = 0;
        c_sin(_mul43r, _mul43i, &_sin44r, &_sin44i);
        double _im45r = 0, _im45i = 0;
        _im45r = x2i; _im45i = 0;
        double _mul46r = 0, _mul46i = 0;
        c_mul(_im45r, _im45i, m, 0, &_mul46r, &_mul46i);
        double _cos47r = 0, _cos47i = 0;
        c_cos(_mul46r, _mul46i, &_cos47r, &_cos47i);
        double _add48r = 0, _add48i = 0;
        _add48r = _sin44r + _cos47r; _add48i = _sin44i + _cos47i;
        cRe[(m - 1)] += _add48r; cIm[(m - 1)] += _add48i;
    }
    double _mul49r = 0, _mul49i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul49r, &_mul49i);
    double _re50r = 0, _re50i = 0;
    _re50r = _mul49r; _re50i = 0;
    double _c51r = 0, _c51i = 0;
    _c51r = 0.0; _c51i = 1.0;
    double _div52r = 0, _div52i = 0;
    c_div(x1r, x1i, x2r, x2i, &_div52r, &_div52i);
    double _im53r = 0, _im53i = 0;
    _im53r = _div52i; _im53i = 0;
    double _mul54r = 0, _mul54i = 0;
    c_mul(_c51r, _c51i, _im53r, _im53i, &_mul54r, &_mul54i);
    double _add55r = 0, _add55i = 0;
    _add55r = _re50r + _mul54r; _add55i = _re50i + _mul54i;
    { int _idx = 11; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add55r; cIm[_idx] = _add55i; } }
    double _add56r = 0, _add56i = 0;
    _add56r = x1r + x2r; _add56i = x1i + x2i;
    double _abs57r = 0, _abs57i = 0;
    _abs57r = c_abs(_add56r, _add56i); _abs57i = 0;
    double _log58r = 0, _log58i = 0;
    c_log(_abs57r, _abs57i, &_log58r, &_log58i);
    double _c59r = 0, _c59i = 0;
    _c59r = 0.0; _c59i = 1.0;
    double _sub60r = 0, _sub60i = 0;
    _sub60r = x1r - x2r; _sub60i = x1i - x2i;
    double _ang61r = 0, _ang61i = 0;
    _ang61r = c_arg(_sub60r, _sub60i); _ang61i = 0;
    double _mul62r = 0, _mul62i = 0;
    c_mul(_c59r, _c59i, _ang61r, _ang61i, &_mul62r, &_mul62i);
    double _add63r = 0, _add63i = 0;
    _add63r = _log58r + _mul62r; _add63i = _log58i + _mul62i;
    { int _idx = 19; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add63r; cIm[_idx] = _add63i; } }
    double _conj64r = 0, _conj64i = 0;
    _conj64r = x1r; _conj64i = -(x1i);
    double _c65r = 0, _c65i = 0;
    _c65r = 2.0; _c65i = 0;
    double _pow66r = 0, _pow66i = 0;
    c_mul(_conj64r, _conj64i, _conj64r, _conj64i, &_pow66r, &_pow66i);
    double _conj67r = 0, _conj67i = 0;
    _conj67r = x2r; _conj67i = -(x2i);
    double _c68r = 0, _c68i = 0;
    _c68r = 3.0; _c68i = 0;
    double _pow69r = 0, _pow69i = 0;
    c_mul(_conj67r, _conj67i, _conj67r, _conj67i, &_pow69r, &_pow69i);
    c_mul(_pow69r, _pow69i, _conj67r, _conj67i, &_pow69r, &_pow69i);
    double _sub70r = 0, _sub70i = 0;
    _sub70r = _pow66r - _pow69r; _sub70i = _pow66i - _pow69i;
    double _mul71r = 0, _mul71i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul71r, &_mul71i);
    double _sin72r = 0, _sin72i = 0;
    c_sin(_mul71r, _mul71i, &_sin72r, &_sin72i);
    double _add73r = 0, _add73i = 0;
    _add73r = _sub70r + _sin72r; _add73i = _sub70i + _sin72i;
    { int _idx = 24; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add73r; cIm[_idx] = _add73i; } }
    double _abs74r = 0, _abs74i = 0;
    _abs74r = c_abs(x1r, x1i); _abs74i = 0;
    double _c75r = 0, _c75i = 0;
    _c75r = 3.0; _c75i = 0;
    double _pow76r = 0, _pow76i = 0;
    c_mul(_abs74r, _abs74i, _abs74r, _abs74i, &_pow76r, &_pow76i);
    c_mul(_pow76r, _pow76i, _abs74r, _abs74i, &_pow76r, &_pow76i);
    double _abs77r = 0, _abs77i = 0;
    _abs77r = c_abs(x2r, x2i); _abs77i = 0;
    double _c78r = 0, _c78i = 0;
    _c78r = 2.0; _c78i = 0;
    double _pow79r = 0, _pow79i = 0;
    c_mul(_abs77r, _abs77i, _abs77r, _abs77i, &_pow79r, &_pow79i);
    double _mul80r = 0, _mul80i = 0;
    c_mul(_pow76r, _pow76i, _pow79r, _pow79i, &_mul80r, &_mul80i);
    double _ang81r = 0, _ang81i = 0;
    _ang81r = c_arg(x1r, x1i); _ang81i = 0;
    double _ang82r = 0, _ang82i = 0;
    _ang82r = c_arg(x2r, x2i); _ang82i = 0;
    double _mul83r = 0, _mul83i = 0;
    c_mul(_ang81r, _ang81i, _ang82r, _ang82i, &_mul83r, &_mul83i);
    double _cos84r = 0, _cos84i = 0;
    c_cos(_mul83r, _mul83i, &_cos84r, &_cos84i);
    double _add85r = 0, _add85i = 0;
    _add85r = _mul80r + _cos84r; _add85i = _mul80i + _cos84i;
    { int _idx = 29; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add85r; cIm[_idx] = _add85i; } }
    double _abs86r = 0, _abs86i = 0;
    _abs86r = c_abs(x1r, x1i); _abs86i = 0;
    double _abs87r = 0, _abs87i = 0;
    _abs87r = c_abs(x2r, x2i); _abs87i = 0;
    double _prod88r = 0, _prod88i = 0;
    c_mul(_abs86r, _abs86i, _abs87r, _abs87i, &_prod88r, &_prod88i);
    double _re89r = 0, _re89i = 0;
    _re89r = x1r; _re89i = 0;
    double _im90r = 0, _im90i = 0;
    _im90r = x2i; _im90i = 0;
    double _sum91r = 0, _sum91i = 0;
    _sum91r = _re89r + _im90r; _sum91i = _re89i + _im90i;
    double _c92r = 0, _c92i = 0;
    _c92r = 0.0; _c92i = 1.0;
    double _mul93r = 0, _mul93i = 0;
    c_mul(_sum91r, _sum91i, _c92r, _c92i, &_mul93r, &_mul93i);
    double _add94r = 0, _add94i = 0;
    _add94r = _prod88r + _mul93r; _add94i = _prod88i + _mul93i;
    { int _idx = 34; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add94r; cIm[_idx] = _add94i; } }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_260_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _len1r = 0, _len1i = 0;
    _len1r = 35; _len1i = 0;
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _add3r = 0, _add3i = 0;
    _add3r = _len1r + _c2r; _add3i = _len1i + _c2i;
    for (int j = 1; j < (int)(_add3r); j++) {
        double _c4r = 0, _c4i = 0;
        _c4r = 7.0; _c4i = 0;
        double _mod5r = 0, _mod5i = 0;
        _mod5r = fmod(j, _c4r); _mod5i = 0;
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = _mod5r + _c6r; _add7i = _mod5i + _c6i;
        double k_r = _add7r, k_i = _add7i;
        double _c8r = 0, _c8i = 0;
        _c8r = 7.0; _c8i = 0;
        double _fdiv9r = 0, _fdiv9i = 0;
        c_div(j, 0, _c8r, _c8i, &_fdiv9r, &_fdiv9i);
        _fdiv9r = floor(_fdiv9r); _fdiv9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _fdiv9r + _c10r; _add11i = _fdiv9i + _c10i;
        double r_r = _add11r, r_i = _add11i;
        double _re12r = 0, _re12i = 0;
        _re12r = x1r; _re12i = 0;
        double _pow13r = 0, _pow13i = 0;
        c_powc(_re12r, _re12i, k_r, k_i, &_pow13r, &_pow13i);
        double _im14r = 0, _im14i = 0;
        _im14r = x2i; _im14i = 0;
        double _pow15r = 0, _pow15i = 0;
        c_powc(_im14r, _im14i, r_r, r_i, &_pow15r, &_pow15i);
        double _sub16r = 0, _sub16i = 0;
        _sub16r = _pow13r - _pow15r; _sub16i = _pow13i - _pow15i;
        double _ang17r = 0, _ang17i = 0;
        _ang17r = c_arg(x1r, x1i); _ang17i = 0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(_ang17r, _ang17i, j, 0, &_mul18r, &_mul18i);
        double _cos19r = 0, _cos19i = 0;
        c_cos(_mul18r, _mul18i, &_cos19r, &_cos19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_sub16r, _sub16i, _cos19r, _cos19i, &_mul20r, &_mul20i);
        double _ang21r = 0, _ang21i = 0;
        _ang21r = c_arg(x2r, x2i); _ang21i = 0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(_ang21r, _ang21i, r_r, r_i, &_mul22r, &_mul22i);
        double _sin23r = 0, _sin23i = 0;
        c_sin(_mul22r, _mul22i, &_sin23r, &_sin23i);
        double _abs24r = 0, _abs24i = 0;
        _abs24r = c_abs(x1r, x1i); _abs24i = 0;
        double _abs25r = 0, _abs25i = 0;
        _abs25r = c_abs(x2r, x2i); _abs25i = 0;
        double _add26r = 0, _add26i = 0;
        _add26r = _abs24r + _abs25r; _add26i = _abs24i + _abs25i;
        double _add27r = 0, _add27i = 0;
        _add27r = _add26r + j; _add27i = _add26i + 0;
        double _div28r = 0, _div28i = 0;
        c_div(_sin23r, _sin23i, _add27r, _add27i, &_div28r, &_div28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _mul20r + _div28r; _add29i = _mul20i + _div28i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add29r; cIm[_idx] = _add29i; } }
    }
    double _conj30r = 0, _conj30i = 0;
    _conj30r = x1r; _conj30i = -(x1i);
    double _c31r = 0, _c31i = 0;
    _c31r = 2.0; _c31i = 0;
    double _pow32r = 0, _pow32i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow32r, &_pow32i);
    double _mul33r = 0, _mul33i = 0;
    c_mul(_conj30r, _conj30i, _pow32r, _pow32i, &_mul33r, &_mul33i);
    double _abs34r = 0, _abs34i = 0;
    _abs34r = c_abs(x1r, x1i); _abs34i = 0;
    double _c35r = 0, _c35i = 0;
    _c35r = 1.0; _c35i = 0;
    double _add36r = 0, _add36i = 0;
    _add36r = _abs34r + _c35r; _add36i = _abs34i + _c35i;
    double _log37r = 0, _log37i = 0;
    c_log(_add36r, _add36i, &_log37r, &_log37i);
    double _sin38r = 0, _sin38i = 0;
    c_sin(x2r, x2i, &_sin38r, &_sin38i);
    double _mul39r = 0, _mul39i = 0;
    c_mul(_log37r, _log37i, _sin38r, _sin38i, &_mul39r, &_mul39i);
    double _add40r = 0, _add40i = 0;
    _add40r = _mul33r + _mul39r; _add40i = _mul33i + _mul39i;
    { int _idx = 3; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add40r; cIm[_idx] = _add40i; } }
    double _mul41r = 0, _mul41i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul41r, &_mul41i);
    double _re42r = 0, _re42i = 0;
    _re42r = _mul41r; _re42i = 0;
    double _im43r = 0, _im43i = 0;
    _im43r = x1i; _im43i = 0;
    double _c44r = 0, _c44i = 0;
    _c44r = 2.0; _c44i = 0;
    double _pow45r = 0, _pow45i = 0;
    c_mul(_im43r, _im43i, _im43r, _im43i, &_pow45r, &_pow45i);
    double _add46r = 0, _add46i = 0;
    _add46r = _re42r + _pow45r; _add46i = _re42i + _pow45i;
    double _cos47r = 0, _cos47i = 0;
    c_cos(x2r, x2i, &_cos47r, &_cos47i);
    double _sub48r = 0, _sub48i = 0;
    _sub48r = _add46r - _cos47r; _sub48i = _add46i - _cos47i;
    { int _idx = 7; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub48r; cIm[_idx] = _sub48i; } }
    double _add49r = 0, _add49i = 0;
    _add49r = x1r + x2r; _add49i = x1i + x2i;
    double _abs50r = 0, _abs50i = 0;
    _abs50r = c_abs(_add49r, _add49i); _abs50i = 0;
    double _c51r = 0, _c51i = 0;
    _c51r = 2.0; _c51i = 0;
    double _pow52r = 0, _pow52i = 0;
    c_mul(_abs50r, _abs50i, _abs50r, _abs50i, &_pow52r, &_pow52i);
    double _re53r = 0, _re53i = 0;
    _re53r = x1r; _re53i = 0;
    double _c54r = 0, _c54i = 0;
    _c54r = 3.0; _c54i = 0;
    double _pow55r = 0, _pow55i = 0;
    c_mul(_re53r, _re53i, _re53r, _re53i, &_pow55r, &_pow55i);
    c_mul(_pow55r, _pow55i, _re53r, _re53i, &_pow55r, &_pow55i);
    double _sub56r = 0, _sub56i = 0;
    _sub56r = _pow52r - _pow55r; _sub56i = _pow52i - _pow55i;
    double _im57r = 0, _im57i = 0;
    _im57r = x2i; _im57i = 0;
    double _add58r = 0, _add58i = 0;
    _add58r = _sub56r + _im57r; _add58i = _sub56i + _im57i;
    { int _idx = 12; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add58r; cIm[_idx] = _add58i; } }
    double _sin59r = 0, _sin59i = 0;
    c_sin(x1r, x1i, &_sin59r, &_sin59i);
    double _cos60r = 0, _cos60i = 0;
    c_cos(x2r, x2i, &_cos60r, &_cos60i);
    double _mul61r = 0, _mul61i = 0;
    c_mul(_sin59r, _sin59i, _cos60r, _cos60i, &_mul61r, &_mul61i);
    double _re62r = 0, _re62i = 0;
    _re62r = x2r; _re62i = 0;
    double _c63r = 0, _c63i = 0;
    _c63r = 2.0; _c63i = 0;
    double _pow64r = 0, _pow64i = 0;
    c_mul(_re62r, _re62i, _re62r, _re62i, &_pow64r, &_pow64i);
    double _add65r = 0, _add65i = 0;
    _add65r = _mul61r + _pow64r; _add65i = _mul61i + _pow64i;
    double _im66r = 0, _im66i = 0;
    _im66r = x1i; _im66i = 0;
    double _c67r = 0, _c67i = 0;
    _c67r = 2.0; _c67i = 0;
    double _pow68r = 0, _pow68i = 0;
    c_mul(_im66r, _im66i, _im66r, _im66i, &_pow68r, &_pow68i);
    double _sub69r = 0, _sub69i = 0;
    _sub69r = _add65r - _pow68r; _sub69i = _add65i - _pow68i;
    { int _idx = 16; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub69r; cIm[_idx] = _sub69i; } }
    double _mul70r = 0, _mul70i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul70r, &_mul70i);
    double _abs71r = 0, _abs71i = 0;
    _abs71r = c_abs(_mul70r, _mul70i); _abs71i = 0;
    double _c72r = 0, _c72i = 0;
    _c72r = 1.0; _c72i = 0;
    double _add73r = 0, _add73i = 0;
    _add73r = _abs71r + _c72r; _add73i = _abs71i + _c72i;
    double _log74r = 0, _log74i = 0;
    c_log(_add73r, _add73i, &_log74r, &_log74i);
    double _conj75r = 0, _conj75i = 0;
    _conj75r = x1r; _conj75i = -(x1i);
    double _add76r = 0, _add76i = 0;
    _add76r = _log74r + _conj75r; _add76i = _log74i + _conj75i;
    double _conj77r = 0, _conj77i = 0;
    _conj77r = x2r; _conj77i = -(x2i);
    double _sub78r = 0, _sub78i = 0;
    _sub78r = _add76r - _conj77r; _sub78i = _add76i - _conj77i;
    { int _idx = 21; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub78r; cIm[_idx] = _sub78i; } }
    double _re79r = 0, _re79i = 0;
    _re79r = x1r; _re79i = 0;
    double _c80r = 0, _c80i = 0;
    _c80r = 2.0; _c80i = 0;
    double _pow81r = 0, _pow81i = 0;
    c_mul(_re79r, _re79i, _re79r, _re79i, &_pow81r, &_pow81i);
    double _im82r = 0, _im82i = 0;
    _im82r = x2i; _im82i = 0;
    double _mul83r = 0, _mul83i = 0;
    c_mul(_pow81r, _pow81i, _im82r, _im82i, &_mul83r, &_mul83i);
    double _re84r = 0, _re84i = 0;
    _re84r = x2r; _re84i = 0;
    double _im85r = 0, _im85i = 0;
    _im85r = x1i; _im85i = 0;
    double _mul86r = 0, _mul86i = 0;
    c_mul(_re84r, _re84i, _im85r, _im85i, &_mul86r, &_mul86i);
    double _sub87r = 0, _sub87i = 0;
    _sub87r = _mul83r - _mul86r; _sub87i = _mul83i - _mul86i;
    double _add88r = 0, _add88i = 0;
    _add88r = x1r + x2r; _add88i = x1i + x2i;
    double _ang89r = 0, _ang89i = 0;
    _ang89r = c_arg(_add88r, _add88i); _ang89i = 0;
    double _sin90r = 0, _sin90i = 0;
    c_sin(_ang89r, _ang89i, &_sin90r, &_sin90i);
    double _add91r = 0, _add91i = 0;
    _add91r = _sub87r + _sin90r; _add91i = _sub87i + _sin90i;
    { int _idx = 25; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add91r; cIm[_idx] = _add91i; } }
    double _re92r = 0, _re92i = 0;
    _re92r = x1r; _re92i = 0;
    double _im93r = 0, _im93i = 0;
    _im93r = x1i; _im93i = 0;
    double _add94r = 0, _add94i = 0;
    _add94r = _re92r + _im93r; _add94i = _re92i + _im93i;
    double _re95r = 0, _re95i = 0;
    _re95r = x2r; _re95i = 0;
    double _im96r = 0, _im96i = 0;
    _im96r = x2i; _im96i = 0;
    double _sub97r = 0, _sub97i = 0;
    _sub97r = _re95r - _im96r; _sub97i = _re95i - _im96i;
    double _mul98r = 0, _mul98i = 0;
    c_mul(_add94r, _add94i, _sub97r, _sub97i, &_mul98r, &_mul98i);
    double _mul99r = 0, _mul99i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul99r, &_mul99i);
    double _ang100r = 0, _ang100i = 0;
    _ang100r = c_arg(_mul99r, _mul99i); _ang100i = 0;
    double _cos101r = 0, _cos101i = 0;
    c_cos(_ang100r, _ang100i, &_cos101r, &_cos101i);
    double _add102r = 0, _add102i = 0;
    _add102r = _mul98r + _cos101r; _add102i = _mul98i + _cos101i;
    { int _idx = 30; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add102r; cIm[_idx] = _add102i; } }
    double _re103r = 0, _re103i = 0;
    _re103r = x1r; _re103i = 0;
    double _c104r = 0, _c104i = 0;
    _c104r = 3.0; _c104i = 0;
    double _pow105r = 0, _pow105i = 0;
    c_mul(_re103r, _re103i, _re103r, _re103i, &_pow105r, &_pow105i);
    c_mul(_pow105r, _pow105i, _re103r, _re103i, &_pow105r, &_pow105i);
    double _im106r = 0, _im106i = 0;
    _im106r = x1i; _im106i = 0;
    double _c107r = 0, _c107i = 0;
    _c107r = 3.0; _c107i = 0;
    double _pow108r = 0, _pow108i = 0;
    c_mul(_im106r, _im106i, _im106r, _im106i, &_pow108r, &_pow108i);
    c_mul(_pow108r, _pow108i, _im106r, _im106i, &_pow108r, &_pow108i);
    double _sub109r = 0, _sub109i = 0;
    _sub109r = _pow105r - _pow108r; _sub109i = _pow105i - _pow108i;
    double _re110r = 0, _re110i = 0;
    _re110r = x2r; _re110i = 0;
    double _c111r = 0, _c111i = 0;
    _c111r = 3.0; _c111i = 0;
    double _pow112r = 0, _pow112i = 0;
    c_mul(_re110r, _re110i, _re110r, _re110i, &_pow112r, &_pow112i);
    c_mul(_pow112r, _pow112i, _re110r, _re110i, &_pow112r, &_pow112i);
    double _add113r = 0, _add113i = 0;
    _add113r = _sub109r + _pow112r; _add113i = _sub109i + _pow112i;
    double _im114r = 0, _im114i = 0;
    _im114r = x2i; _im114i = 0;
    double _c115r = 0, _c115i = 0;
    _c115r = 3.0; _c115i = 0;
    double _pow116r = 0, _pow116i = 0;
    c_mul(_im114r, _im114i, _im114r, _im114i, &_pow116r, &_pow116i);
    c_mul(_pow116r, _pow116i, _im114r, _im114i, &_pow116r, &_pow116i);
    double _sub117r = 0, _sub117i = 0;
    _sub117r = _add113r - _pow116r; _sub117i = _add113i - _pow116i;
    { int _idx = 33; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub117r; cIm[_idx] = _sub117i; } }
    double _re118r = 0, _re118i = 0;
    _re118r = x1r; _re118i = 0;
    double _re119r = 0, _re119i = 0;
    _re119r = x2r; _re119i = 0;
    double _sum120r = 0, _sum120i = 0;
    _sum120r = _re118r + _re119r; _sum120i = _re118i + _re119i;
    double _im121r = 0, _im121i = 0;
    _im121r = x1i; _im121i = 0;
    double _sum122r = 0, _sum122i = 0;
    _sum122r = _sum120r + _im121r; _sum122i = _sum120i + _im121i;
    double _im123r = 0, _im123i = 0;
    _im123r = x2i; _im123i = 0;
    double _sum124r = 0, _sum124i = 0;
    _sum124r = _sum122r + _im123r; _sum124i = _sum122i + _im123i;
    double _abs125r = 0, _abs125i = 0;
    _abs125r = c_abs(x1r, x1i); _abs125i = 0;
    double _abs126r = 0, _abs126i = 0;
    _abs126r = c_abs(x2r, x2i); _abs126i = 0;
    double _prod127r = 0, _prod127i = 0;
    c_mul(_abs125r, _abs125i, _abs126r, _abs126i, &_prod127r, &_prod127i);
    double _add128r = 0, _add128i = 0;
    _add128r = _sum124r + _prod127r; _add128i = _sum124i + _prod127i;
    { int _idx = 34; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add128r; cIm[_idx] = _add128i; } }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_261_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 36; j++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 1.0; _c1i = 0;
        double _abs2r = 0, _abs2i = 0;
        _abs2r = c_abs(x1r, x1i); _abs2i = 0;
        double _pow3r = 0, _pow3i = 0;
        c_powr(_abs2r, _abs2i, j, &_pow3r, &_pow3i);
        double _add4r = 0, _add4i = 0;
        _add4r = _c1r + _pow3r; _add4i = _c1i + _pow3i;
        double _abs5r = 0, _abs5i = 0;
        _abs5r = c_abs(x2r, x2i); _abs5i = 0;
        double _c6r = 0, _c6i = 0;
        _c6r = 35.0; _c6i = 0;
        double _sub7r = 0, _sub7i = 0;
        _sub7r = _c6r - j; _sub7i = _c6i - 0;
        double _pow8r = 0, _pow8i = 0;
        c_powc(_abs5r, _abs5i, _sub7r, _sub7i, &_pow8r, &_pow8i);
        double _add9r = 0, _add9i = 0;
        _add9r = _add4r + _pow8r; _add9i = _add4i + _pow8i;
        double _log10r = 0, _log10i = 0;
        c_log(_add9r, _add9i, &_log10r, &_log10i);
        double _ang11r = 0, _ang11i = 0;
        _ang11r = c_arg(x1r, x1i); _ang11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(j, 0, _ang11r, _ang11i, &_mul12r, &_mul12i);
        double _ang13r = 0, _ang13i = 0;
        _ang13r = c_arg(x2r, x2i); _ang13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = _mul12r + _ang13r; _add14i = _mul12i + _ang13i;
        double _sin15r = 0, _sin15i = 0;
        c_sin(_add14r, _add14i, &_sin15r, &_sin15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _log10r + _sin15r; _add16i = _log10i + _sin15i;
        double mag_r = _add16r, mag_i = _add16i;
        double _ang17r = 0, _ang17i = 0;
        _ang17r = c_arg(x1r, x1i); _ang17i = 0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(j, 0, _ang17r, _ang17i, &_mul18r, &_mul18i);
        double _cos19r = 0, _cos19i = 0;
        c_cos(_mul18r, _mul18i, &_cos19r, &_cos19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 35.0; _c20i = 0;
        double _sub21r = 0, _sub21i = 0;
        _sub21r = _c20r - j; _sub21i = _c20i - 0;
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x2r, x2i); _ang22i = 0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(_sub21r, _sub21i, _ang22r, _ang22i, &_mul23r, &_mul23i);
        double _sin24r = 0, _sin24i = 0;
        c_sin(_mul23r, _mul23i, &_sin24r, &_sin24i);
        double _sub25r = 0, _sub25i = 0;
        _sub25r = _cos19r - _sin24r; _sub25i = _cos19i - _sin24i;
        double angle_r = _sub25r, angle_i = _sub25i;
        double _cos26r = 0, _cos26i = 0;
        c_cos(angle_r, angle_i, &_cos26r, &_cos26i);
        double _c27r = 0, _c27i = 0;
        _c27r = 0.0; _c27i = 1.0;
        double _sin28r = 0, _sin28i = 0;
        c_sin(angle_r, angle_i, &_sin28r, &_sin28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_c27r, _c27i, _sin28r, _sin28i, &_mul29r, &_mul29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _cos26r + _mul29r; _add30i = _cos26i + _mul29i;
        double _mul31r = 0, _mul31i = 0;
        c_mul(mag_r, mag_i, _add30r, _add30i, &_mul31r, &_mul31i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul31r; cIm[_idx] = _mul31i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_262: auto-stubbed (unhandled constructs in source) */
static void poly_262_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_263: auto-stubbed (unhandled constructs in source) */
static void poly_263_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_264_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 1; k < 36; k++) {
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
        c_mul(_ang5r, _ang5i, k, 0, &_mul6r, &_mul6i);
        double _sin7r = 0, _sin7i = 0;
        c_sin(_mul6r, _mul6i, &_sin7r, &_sin7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(_log4r, _log4i, _sin7r, _sin7i, &_mul8r, &_mul8i);
        double _ang9r = 0, _ang9i = 0;
        _ang9r = c_arg(x1r, x1i); _ang9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(_ang9r, _ang9i, k, 0, &_mul10r, &_mul10i);
        double _cos11r = 0, _cos11i = 0;
        c_cos(_mul10r, _mul10i, &_cos11r, &_cos11i);
        double _add12r = 0, _add12i = 0;
        _add12r = _mul8r + _cos11r; _add12i = _mul8i + _cos11i;
        double r_r = _add12r, r_i = _add12i;
        double _ang13r = 0, _ang13i = 0;
        _ang13r = c_arg(x1r, x1i); _ang13i = 0;
        double _c14r = 0, _c14i = 0;
        _c14r = 2.0; _c14i = 0;
        double _pow15r = 0, _pow15i = 0;
        c_mul(_ang13r, _ang13i, _ang13r, _ang13i, &_pow15r, &_pow15i);
        double _c16r = 0, _c16i = 0;
        _c16r = 1.0; _c16i = 0;
        double _add17r = 0, _add17i = 0;
        _add17r = k + _c16r; _add17i = 0 + _c16i;
        double _div18r = 0, _div18i = 0;
        c_div(_pow15r, _pow15i, _add17r, _add17i, &_div18r, &_div18i);
        double _ang19r = 0, _ang19i = 0;
        _ang19r = c_arg(x2r, x2i); _ang19i = 0;
        double _abs20r = 0, _abs20i = 0;
        _abs20r = c_abs(x2r, x2i); _abs20i = 0;
        double _c21r = 0, _c21i = 0;
        _c21r = 1.0; _c21i = 0;
        double _add22r = 0, _add22i = 0;
        _add22r = _abs20r + _c21r; _add22i = _abs20i + _c21i;
        double _log23r = 0, _log23i = 0;
        c_log(_add22r, _add22i, &_log23r, &_log23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_ang19r, _ang19i, _log23r, _log23i, &_mul24r, &_mul24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _div18r + _mul24r; _add25i = _div18i + _mul24i;
        double theta_r = _add25r, theta_i = _add25i;
        double _re26r = 0, _re26i = 0;
        _re26r = x1r; _re26i = 0;
        double _im27r = 0, _im27i = 0;
        _im27r = x2i; _im27i = 0;
        double _add28r = 0, _add28i = 0;
        _add28r = _re26r + _im27r; _add28i = _re26i + _im27i;
        double _pow29r = 0, _pow29i = 0;
        c_powr(_add28r, _add28i, k, &_pow29r, &_pow29i);
        double _c30r = 0, _c30i = 0;
        _c30r = 2.0; _c30i = 0;
        double _add31r = 0, _add31i = 0;
        _add31r = k + _c30r; _add31i = 0 + _c30i;
        double _div32r = 0, _div32i = 0;
        c_div(_pow29r, _pow29i, _add31r, _add31i, &_div32r, &_div32i);
        double _re33r = 0, _re33i = 0;
        _re33r = x2r; _re33i = 0;
        double _im34r = 0, _im34i = 0;
        _im34r = x1i; _im34i = 0;
        double _sub35r = 0, _sub35i = 0;
        _sub35r = _re33r - _im34r; _sub35i = _re33i - _im34i;
        double _c36r = 0, _c36i = 0;
        _c36r = 5.0; _c36i = 0;
        double _mod37r = 0, _mod37i = 0;
        _mod37r = fmod(k, _c36r); _mod37i = 0;
        double _c38r = 0, _c38i = 0;
        _c38r = 1.0; _c38i = 0;
        double _add39r = 0, _add39i = 0;
        _add39r = _mod37r + _c38r; _add39i = _mod37i + _c38i;
        double _pow40r = 0, _pow40i = 0;
        c_powc(_sub35r, _sub35i, _add39r, _add39i, &_pow40r, &_pow40i);
        double _add41r = 0, _add41i = 0;
        _add41r = _div32r + _pow40r; _add41i = _div32i + _pow40i;
        double magnitude_r = _add41r, magnitude_i = _add41i;
        double _cos42r = 0, _cos42i = 0;
        c_cos(theta_r, theta_i, &_cos42r, &_cos42i);
        double _sin43r = 0, _sin43i = 0;
        c_sin(theta_r, theta_i, &_sin43r, &_sin43i);
        double _c44r = 0, _c44i = 0;
        _c44r = 0.0; _c44i = 1.0;
        double _mul45r = 0, _mul45i = 0;
        c_mul(_sin43r, _sin43i, _c44r, _c44i, &_mul45r, &_mul45i);
        double _add46r = 0, _add46i = 0;
        _add46r = _cos42r + _mul45r; _add46i = _cos42i + _mul45i;
        double _mul47r = 0, _mul47i = 0;
        c_mul(magnitude_r, magnitude_i, _add46r, _add46i, &_mul47r, &_mul47i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul47r; cIm[_idx] = _mul47i; } }
    }
    for (int j = 5; j < 36; j += 5) {
        double _cf48r = 0, _cf48i = 0;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { _cf48r = cRe[_idx]; _cf48i = cIm[_idx]; } }
        double _conj49r = 0, _conj49i = 0;
        _conj49r = _cf48r; _conj49i = -(_cf48i);
        double _c50r = 0, _c50i = 0;
        _c50r = 2.0; _c50i = 0;
        double _pow51r = 0, _pow51i = 0;
        c_mul(x1r, x1i, x1r, x1i, &_pow51r, &_pow51i);
        double _mul52r = 0, _mul52i = 0;
        c_mul(_conj49r, _conj49i, _pow51r, _pow51i, &_mul52r, &_mul52i);
        double _c53r = 0, _c53i = 0;
        _c53r = 3.0; _c53i = 0;
        double _pow54r = 0, _pow54i = 0;
        c_mul(x2r, x2i, x2r, x2i, &_pow54r, &_pow54i);
        c_mul(_pow54r, _pow54i, x2r, x2i, &_pow54r, &_pow54i);
        double _sub55r = 0, _sub55i = 0;
        _sub55r = _mul52r - _pow54r; _sub55i = _mul52i - _pow54i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub55r; cIm[_idx] = _sub55i; } }
    }
    for (int r = 3; r < 36; r += 3) {
        double _re56r = 0, _re56i = 0;
        _re56r = x1r; _re56i = 0;
        double _cf57r = 0, _cf57i = 0;
        { int _idx = ((int)(r_r) - 1); if (_idx >= 0 && _idx < 35) { _cf57r = cRe[_idx]; _cf57i = cIm[_idx]; } }
        double _mul58r = 0, _mul58i = 0;
        c_mul(_re56r, _re56i, _cf57r, _cf57i, &_mul58r, &_mul58i);
        double _im59r = 0, _im59i = 0;
        _im59r = x2i; _im59i = 0;
        double _cf60r = 0, _cf60i = 0;
        { int _idx = ((int)(r_r) - 1); if (_idx >= 0 && _idx < 35) { _cf60r = cRe[_idx]; _cf60i = cIm[_idx]; } }
        double _c61r = 0, _c61i = 0;
        _c61r = 2.0; _c61i = 0;
        double _pow62r = 0, _pow62i = 0;
        c_mul(_cf60r, _cf60i, _cf60r, _cf60i, &_pow62r, &_pow62i);
        double _mul63r = 0, _mul63i = 0;
        c_mul(_im59r, _im59i, _pow62r, _pow62i, &_mul63r, &_mul63i);
        double _add64r = 0, _add64i = 0;
        _add64r = _mul58r + _mul63r; _add64i = _mul58i + _mul63i;
        { int _idx = ((int)(r_r) - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add64r; cIm[_idx] = _add64i; } }
    }
    double _c65r = 0, _c65i = 0;
    _c65r = 1.0; _c65i = 0;
    double _add66r = 0, _add66i = 0;
    _add66r = _c65r + x1r; _add66i = _c65i + x1i;
    double _sub67r = 0, _sub67i = 0;
    _sub67r = _add66r - x2r; _sub67i = _add66i - x2i;
    { int _idx = 0; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub67r; cIm[_idx] = _sub67i; } }
    double _mul68r = 0, _mul68i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul68r, &_mul68i);
    double _sin69r = 0, _sin69i = 0;
    c_sin(_mul68r, _mul68i, &_sin69r, &_sin69i);
    double _abs70r = 0, _abs70i = 0;
    _abs70r = c_abs(x2r, x2i); _abs70i = 0;
    double _c71r = 0, _c71i = 0;
    _c71r = 1.0; _c71i = 0;
    double _add72r = 0, _add72i = 0;
    _add72r = _abs70r + _c71r; _add72i = _abs70i + _c71i;
    double _div73r = 0, _div73i = 0;
    c_div(x1r, x1i, _add72r, _add72i, &_div73r, &_div73i);
    double _cos74r = 0, _cos74i = 0;
    c_cos(_div73r, _div73i, &_cos74r, &_cos74i);
    double _add75r = 0, _add75i = 0;
    _add75r = _sin69r + _cos74r; _add75i = _sin69i + _cos74i;
    { int _idx = 34; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add75r; cIm[_idx] = _add75i; } }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_265_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 36; j++) {
        double _ang1r = 0, _ang1i = 0;
        _ang1r = c_arg(x1r, x1i); _ang1i = 0;
        double _mul2r = 0, _mul2i = 0;
        c_mul(_ang1r, _ang1i, j, 0, &_mul2r, &_mul2i);
        double _ang3r = 0, _ang3i = 0;
        _ang3r = c_arg(x2r, x2i); _ang3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 35.0; _c4i = 0;
        double _sub5r = 0, _sub5i = 0;
        _sub5r = _c4r - j; _sub5i = _c4i - 0;
        double _mul6r = 0, _mul6i = 0;
        c_mul(_ang3r, _ang3i, _sub5r, _sub5i, &_mul6r, &_mul6i);
        double _add7r = 0, _add7i = 0;
        _add7r = _mul2r + _mul6r; _add7i = _mul2i + _mul6i;
        double angle_r = _add7r, angle_i = _add7i;
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(x1r, x1i); _abs8i = 0;
        double _c9r = 0, _c9i = 0;
        _c9r = 5.0; _c9i = 0;
        double _mod10r = 0, _mod10i = 0;
        _mod10r = fmod(j, _c9r); _mod10i = 0;
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = _mod10r + _c11r; _add12i = _mod10i + _c11i;
        double _pow13r = 0, _pow13i = 0;
        c_powc(_abs8r, _abs8i, _add12r, _add12i, &_pow13r, &_pow13i);
        double _abs14r = 0, _abs14i = 0;
        _abs14r = c_abs(x2r, x2i); _abs14i = 0;
        double _c15r = 0, _c15i = 0;
        _c15r = 7.0; _c15i = 0;
        double _mod16r = 0, _mod16i = 0;
        _mod16r = fmod(j, _c15r); _mod16i = 0;
        double _c17r = 0, _c17i = 0;
        _c17r = 1.0; _c17i = 0;
        double _add18r = 0, _add18i = 0;
        _add18r = _mod16r + _c17r; _add18i = _mod16i + _c17i;
        double _pow19r = 0, _pow19i = 0;
        c_powc(_abs14r, _abs14i, _add18r, _add18i, &_pow19r, &_pow19i);
        double _add20r = 0, _add20i = 0;
        _add20r = _pow13r + _pow19r; _add20i = _pow13i + _pow19i;
        double _mul21r = 0, _mul21i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul21r, &_mul21i);
        double _abs22r = 0, _abs22i = 0;
        _abs22r = c_abs(_mul21r, _mul21i); _abs22i = 0;
        double _c23r = 0, _c23i = 0;
        _c23r = 1.0; _c23i = 0;
        double _add24r = 0, _add24i = 0;
        _add24r = _abs22r + _c23r; _add24i = _abs22i + _c23i;
        double _log25r = 0, _log25i = 0;
        c_log(_add24r, _add24i, &_log25r, &_log25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _add20r + _log25r; _add26i = _add20i + _log25i;
        double magnitude_r = _add26r, magnitude_i = _add26i;
        double _re27r = 0, _re27i = 0;
        _re27r = x1r; _re27i = 0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(j, 0, _re27r, _re27i, &_mul28r, &_mul28i);
        double _sin29r = 0, _sin29i = 0;
        c_sin(_mul28r, _mul28i, &_sin29r, &_sin29i);
        double _im30r = 0, _im30i = 0;
        _im30r = x2i; _im30i = 0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(j, 0, _im30r, _im30i, &_mul31r, &_mul31i);
        double _cos32r = 0, _cos32i = 0;
        c_cos(_mul31r, _mul31i, &_cos32r, &_cos32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _sin29r + _cos32r; _add33i = _sin29i + _cos32i;
        double _add34r = 0, _add34i = 0;
        _add34r = x1r + x2r; _add34i = x1i + x2i;
        double _ang35r = 0, _ang35i = 0;
        _ang35r = c_arg(_add34r, _add34i); _ang35i = 0;
        double _add36r = 0, _add36i = 0;
        _add36r = _add33r + _ang35r; _add36i = _add33i + _ang35i;
        double phase_r = _add36r, phase_i = _add36i;
        double _cos37r = 0, _cos37i = 0;
        c_cos(phase_r, phase_i, &_cos37r, &_cos37i);
        double _c38r = 0, _c38i = 0;
        _c38r = 0.0; _c38i = 1.0;
        double _sin39r = 0, _sin39i = 0;
        c_sin(phase_r, phase_i, &_sin39r, &_sin39i);
        double _mul40r = 0, _mul40i = 0;
        c_mul(_c38r, _c38i, _sin39r, _sin39i, &_mul40r, &_mul40i);
        double _add41r = 0, _add41i = 0;
        _add41r = _cos37r + _mul40r; _add41i = _cos37i + _mul40i;
        double _mul42r = 0, _mul42i = 0;
        c_mul(magnitude_r, magnitude_i, _add41r, _add41i, &_mul42r, &_mul42i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul42r; cIm[_idx] = _mul42i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_266_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 1; k < 36; k++) {
        double _abs1r = 0, _abs1i = 0;
        _abs1r = c_abs(x1r, x1i); _abs1i = 0;
        double _c2r = 0, _c2i = 0;
        _c2r = 2.0; _c2i = 0;
        double _pow3r = 0, _pow3i = 0;
        c_mul(k, 0, k, 0, &_pow3r, &_pow3i);
        double _mul4r = 0, _mul4i = 0;
        c_mul(_abs1r, _abs1i, _pow3r, _pow3i, &_mul4r, &_mul4i);
        double _sin5r = 0, _sin5i = 0;
        c_sin(_mul4r, _mul4i, &_sin5r, &_sin5i);
        double _abs6r = 0, _abs6i = 0;
        _abs6r = c_abs(x2r, x2i); _abs6i = 0;
        double _div7r = 0, _div7i = 0;
        c_div(_abs6r, _abs6i, k, 0, &_div7r, &_div7i);
        double _cos8r = 0, _cos8i = 0;
        c_cos(_div7r, _div7i, &_cos8r, &_cos8i);
        double _add9r = 0, _add9i = 0;
        _add9r = _sin5r + _cos8r; _add9i = _sin5i + _cos8i;
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(x1r, x1i); _abs10i = 0;
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = _abs10r + _c11r; _add12i = _abs10i + _c11i;
        double _log13r = 0, _log13i = 0;
        c_log(_add12r, _add12i, &_log13r, &_log13i);
        double _abs14r = 0, _abs14i = 0;
        _abs14r = c_abs(x2r, x2i); _abs14i = 0;
        double _c15r = 0, _c15i = 0;
        _c15r = 1.0; _c15i = 0;
        double _add16r = 0, _add16i = 0;
        _add16r = _abs14r + _c15r; _add16i = _abs14i + _c15i;
        double _log17r = 0, _log17i = 0;
        c_log(_add16r, _add16i, &_log17r, &_log17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_log13r, _log13i, _log17r, _log17i, &_mul18r, &_mul18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _add9r + _mul18r; _add19i = _add9i + _mul18i;
        double mag_r = _add19r, mag_i = _add19i;
        double _ang20r = 0, _ang20i = 0;
        _ang20r = c_arg(x1r, x1i); _ang20i = 0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(_ang20r, _ang20i, k, 0, &_mul21r, &_mul21i);
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x2r, x2i); _ang22i = 0;
        double _c23r = 0, _c23i = 0;
        _c23r = 35.0; _c23i = 0;
        double _sub24r = 0, _sub24i = 0;
        _sub24r = _c23r - k; _sub24i = _c23i - 0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(_ang22r, _ang22i, _sub24r, _sub24i, &_mul25r, &_mul25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _mul21r + _mul25r; _add26i = _mul21i + _mul25i;
        double _sin27r = 0, _sin27i = 0;
        c_sin(k, 0, &_sin27r, &_sin27i);
        double _cos28r = 0, _cos28i = 0;
        c_cos(k, 0, &_cos28r, &_cos28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_sin27r, _sin27i, _cos28r, _cos28i, &_mul29r, &_mul29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _add26r + _mul29r; _add30i = _add26i + _mul29i;
        double ang_r = _add30r, ang_i = _add30i;
        double _cos31r = 0, _cos31i = 0;
        c_cos(ang_r, ang_i, &_cos31r, &_cos31i);
        double _c32r = 0, _c32i = 0;
        _c32r = 0.0; _c32i = 1.0;
        double _sin33r = 0, _sin33i = 0;
        c_sin(ang_r, ang_i, &_sin33r, &_sin33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(_c32r, _c32i, _sin33r, _sin33i, &_mul34r, &_mul34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _cos31r + _mul34r; _add35i = _cos31i + _mul34i;
        double _mul36r = 0, _mul36i = 0;
        c_mul(mag_r, mag_i, _add35r, _add35i, &_mul36r, &_mul36i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul36r; cIm[_idx] = _mul36i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_267_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 36; j++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 3.0; _c1i = 0;
        double _mul2r = 0, _mul2i = 0;
        c_mul(j, 0, _c1r, _c1i, &_mul2r, &_mul2i);
        double _c3r = 0, _c3i = 0;
        _c3r = 7.0; _c3i = 0;
        double _add4r = 0, _add4i = 0;
        _add4r = _mul2r + _c3r; _add4i = _mul2i + _c3i;
        double _c5r = 0, _c5i = 0;
        _c5r = 35.0; _c5i = 0;
        double _mod6r = 0, _mod6i = 0;
        _mod6r = fmod(_add4r, _c5r); _mod6i = 0;
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _mod6r + _c7r; _add8i = _mod6i + _c7i;
        double k_r = _add8r, k_i = _add8i;
        double _ang9r = 0, _ang9i = 0;
        _ang9r = c_arg(x1r, x1i); _ang9i = 0;
        double _sin10r = 0, _sin10i = 0;
        c_sin(j, 0, &_sin10r, &_sin10i);
        double _mul11r = 0, _mul11i = 0;
        c_mul(_ang9r, _ang9i, _sin10r, _sin10i, &_mul11r, &_mul11i);
        double _ang12r = 0, _ang12i = 0;
        _ang12r = c_arg(x2r, x2i); _ang12i = 0;
        double _cos13r = 0, _cos13i = 0;
        c_cos(j, 0, &_cos13r, &_cos13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_ang12r, _ang12i, _cos13r, _cos13i, &_mul14r, &_mul14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _mul11r + _mul14r; _add15i = _mul11i + _mul14i;
        double angle_r = _add15r, angle_i = _add15i;
        double _abs16r = 0, _abs16i = 0;
        _abs16r = c_abs(x1r, x1i); _abs16i = 0;
        double _c17r = 0, _c17i = 0;
        _c17r = 1.0; _c17i = 0;
        double _add18r = 0, _add18i = 0;
        _add18r = _abs16r + _c17r; _add18i = _abs16i + _c17i;
        double _log19r = 0, _log19i = 0;
        c_log(_add18r, _add18i, &_log19r, &_log19i);
        double _re20r = 0, _re20i = 0;
        _re20r = x2r; _re20i = 0;
        double _c21r = 0, _c21i = 0;
        _c21r = 0.5; _c21i = 0;
        double _pow22r = 0, _pow22i = 0;
        c_powr(_re20r, _re20i, 0.5, &_pow22r, &_pow22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_log19r, _log19i, _pow22r, _pow22i, &_mul23r, &_mul23i);
        double _im24r = 0, _im24i = 0;
        _im24r = x1i; _im24i = 0;
        double _c25r = 0, _c25i = 0;
        _c25r = 2.0; _c25i = 0;
        double _pow26r = 0, _pow26i = 0;
        c_mul(_im24r, _im24i, _im24r, _im24i, &_pow26r, &_pow26i);
        double _c27r = 0, _c27i = 0;
        _c27r = 1.0; _c27i = 0;
        double _add28r = 0, _add28i = 0;
        _add28r = j + _c27r; _add28i = 0 + _c27i;
        double _div29r = 0, _div29i = 0;
        c_div(_pow26r, _pow26i, _add28r, _add28i, &_div29r, &_div29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _mul23r + _div29r; _add30i = _mul23i + _div29i;
        double magnitude_r = _add30r, magnitude_i = _add30i;
        double _cos31r = 0, _cos31i = 0;
        c_cos(angle_r, angle_i, &_cos31r, &_cos31i);
        double _sin32r = 0, _sin32i = 0;
        c_sin(angle_r, angle_i, &_sin32r, &_sin32i);
        double _c33r = 0, _c33i = 0;
        _c33r = 0.0; _c33i = 1.0;
        double _mul34r = 0, _mul34i = 0;
        c_mul(_sin32r, _sin32i, _c33r, _c33i, &_mul34r, &_mul34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _cos31r + _mul34r; _add35i = _cos31i + _mul34i;
        double _mul36r = 0, _mul36i = 0;
        c_mul(magnitude_r, magnitude_i, _add35r, _add35i, &_mul36r, &_mul36i);
        double _conj37r = 0, _conj37i = 0;
        _conj37r = x1r; _conj37i = -(x1i);
        double _pow38r = 0, _pow38i = 0;
        c_powr(x2r, x2i, j, &_pow38r, &_pow38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(_conj37r, _conj37i, _pow38r, _pow38i, &_mul39r, &_mul39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _mul36r + _mul39r; _add40i = _mul36i + _mul39i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add40r; cIm[_idx] = _add40i; } }
    }
    double _re41r = 0, _re41i = 0;
    _re41r = x1r; _re41i = 0;
    double _im42r = 0, _im42i = 0;
    _im42r = x2i; _im42i = 0;
    double _c43r = 0, _c43i = 0;
    _c43r = 0.0; _c43i = 1.0;
    double _mul44r = 0, _mul44i = 0;
    c_mul(_im42r, _im42i, _c43r, _c43i, &_mul44r, &_mul44i);
    double _add45r = 0, _add45i = 0;
    _add45r = _re41r + _mul44r; _add45i = _re41i + _mul44i;
    { int _idx = 4; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add45r; cIm[_idx] = _add45i; } }
    double _abs46r = 0, _abs46i = 0;
    _abs46r = c_abs(x1r, x1i); _abs46i = 0;
    double _c47r = 0, _c47i = 0;
    _c47r = 2.0; _c47i = 0;
    double _pow48r = 0, _pow48i = 0;
    c_mul(_abs46r, _abs46i, _abs46r, _abs46i, &_pow48r, &_pow48i);
    double _abs49r = 0, _abs49i = 0;
    _abs49r = c_abs(x2r, x2i); _abs49i = 0;
    double _c50r = 0, _c50i = 0;
    _c50r = 2.0; _c50i = 0;
    double _pow51r = 0, _pow51i = 0;
    c_mul(_abs49r, _abs49i, _abs49r, _abs49i, &_pow51r, &_pow51i);
    double _sub52r = 0, _sub52i = 0;
    _sub52r = _pow48r - _pow51r; _sub52i = _pow48i - _pow51i;
    double _re53r = 0, _re53i = 0;
    _re53r = x1r; _re53i = 0;
    double _im54r = 0, _im54i = 0;
    _im54r = x2i; _im54i = 0;
    double _mul55r = 0, _mul55i = 0;
    c_mul(_re53r, _re53i, _im54r, _im54i, &_mul55r, &_mul55i);
    double _c56r = 0, _c56i = 0;
    _c56r = 0.0; _c56i = 1.0;
    double _mul57r = 0, _mul57i = 0;
    c_mul(_mul55r, _mul55i, _c56r, _c56i, &_mul57r, &_mul57i);
    double _add58r = 0, _add58i = 0;
    _add58r = _sub52r + _mul57r; _add58i = _sub52i + _mul57i;
    { int _idx = 11; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add58r; cIm[_idx] = _add58i; } }
    double _sin59r = 0, _sin59i = 0;
    c_sin(x1r, x1i, &_sin59r, &_sin59i);
    double _cos60r = 0, _cos60i = 0;
    c_cos(x2r, x2i, &_cos60r, &_cos60i);
    double _c61r = 0, _c61i = 0;
    _c61r = 0.0; _c61i = 1.0;
    double _mul62r = 0, _mul62i = 0;
    c_mul(_cos60r, _cos60i, _c61r, _c61i, &_mul62r, &_mul62i);
    double _add63r = 0, _add63i = 0;
    _add63r = _sin59r + _mul62r; _add63i = _sin59i + _mul62i;
    { int _idx = 19; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add63r; cIm[_idx] = _add63i; } }
    for (int r = 25; r < 36; r++) {
        double _prod64r = 0, _prod64i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_prod64r, &_prod64i);
        double _pow65r = 0, _pow65i = 0;
        c_powr(_prod64r, _prod64i, r, &_pow65r, &_pow65i);
        double _c66r = 0, _c66i = 0;
        _c66r = 1.0; _c66i = 0;
        double _add67r = 0, _add67i = 0;
        _add67r = r + _c66r; _add67i = 0 + _c66i;
        double _div68r = 0, _div68i = 0;
        c_div(_pow65r, _pow65i, _add67r, _add67i, &_div68r, &_div68i);
        cRe[(r - 1)] += _div68r; cIm[(r - 1)] += _div68i;
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_268_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 36; j++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 2.0; _c1i = 0;
        double _add2r = 0, _add2i = 0;
        _add2r = j + _c1r; _add2i = 0 + _c1i;
        double k_r = _add2r, k_i = _add2i;
        double _c3r = 0, _c3i = 0;
        _c3r = 5.0; _c3i = 0;
        double _mod4r = 0, _mod4i = 0;
        _mod4r = fmod(j, _c3r); _mod4i = 0;
        double _c5r = 0, _c5i = 0;
        _c5r = 1.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = _mod4r + _c5r; _add6i = _mod4i + _c5i;
        double r_r = _add6r, r_i = _add6i;
        double _re7r = 0, _re7i = 0;
        _re7r = x1r; _re7i = 0;
        double _pow8r = 0, _pow8i = 0;
        c_powc(_re7r, _re7i, k_r, k_i, &_pow8r, &_pow8i);
        double _im9r = 0, _im9i = 0;
        _im9r = x2i; _im9i = 0;
        double _pow10r = 0, _pow10i = 0;
        c_powc(_im9r, _im9i, r_r, r_i, &_pow10r, &_pow10i);
        double _sub11r = 0, _sub11i = 0;
        _sub11r = _pow8r - _pow10r; _sub11i = _pow8i - _pow10i;
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(x1r, x1i); _abs12i = 0;
        double _mul13r = 0, _mul13i = 0;
        c_mul(_abs12r, _abs12i, j, 0, &_mul13r, &_mul13i);
        double _sin14r = 0, _sin14i = 0;
        c_sin(_mul13r, _mul13i, &_sin14r, &_sin14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_sub11r, _sub11i, _sin14r, _sin14i, &_mul15r, &_mul15i);
        double _ang16r = 0, _ang16i = 0;
        _ang16r = c_arg(x2r, x2i); _ang16i = 0;
        double _add17r = 0, _add17i = 0;
        _add17r = _ang16r + j; _add17i = _ang16i + 0;
        double _abs18r = 0, _abs18i = 0;
        _abs18r = c_abs(x1r, x1i); _abs18i = 0;
        double _c19r = 0, _c19i = 0;
        _c19r = 1.0; _c19i = 0;
        double _add20r = 0, _add20i = 0;
        _add20r = _abs18r + _c19r; _add20i = _abs18i + _c19i;
        double _log21r = 0, _log21i = 0;
        c_log(_add20r, _add20i, &_log21r, &_log21i);
        double _cos22r = 0, _cos22i = 0;
        c_cos(_log21r, _log21i, &_cos22r, &_cos22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_add17r, _add17i, _cos22r, _cos22i, &_mul23r, &_mul23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _mul15r + _mul23r; _add24i = _mul15i + _mul23i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add24r; cIm[_idx] = _add24i; } }
    }
    double _conj25r = 0, _conj25i = 0;
    _conj25r = x1r; _conj25i = -(x1i);
    double _c26r = 0, _c26i = 0;
    _c26r = 2.0; _c26i = 0;
    double _pow27r = 0, _pow27i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow27r, &_pow27i);
    double _mul28r = 0, _mul28i = 0;
    c_mul(_conj25r, _conj25i, _pow27r, _pow27i, &_mul28r, &_mul28i);
    double _abs29r = 0, _abs29i = 0;
    _abs29r = c_abs(x2r, x2i); _abs29i = 0;
    double _cos30r = 0, _cos30i = 0;
    c_cos(x1r, x1i, &_cos30r, &_cos30i);
    double _mul31r = 0, _mul31i = 0;
    c_mul(_abs29r, _abs29i, _cos30r, _cos30i, &_mul31r, &_mul31i);
    double _sub32r = 0, _sub32i = 0;
    _sub32r = _mul28r - _mul31r; _sub32i = _mul28i - _mul31i;
    { int _idx = 3; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub32r; cIm[_idx] = _sub32i; } }
    double _mul33r = 0, _mul33i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul33r, &_mul33i);
    double _sin34r = 0, _sin34i = 0;
    c_sin(_mul33r, _mul33i, &_sin34r, &_sin34i);
    double _add35r = 0, _add35i = 0;
    _add35r = x1r + x2r; _add35i = x1i + x2i;
    double _cos36r = 0, _cos36i = 0;
    c_cos(_add35r, _add35i, &_cos36r, &_cos36i);
    double _mul37r = 0, _mul37i = 0;
    c_mul(_cos36r, _cos36i, x1r, x1i, &_mul37r, &_mul37i);
    double _add38r = 0, _add38i = 0;
    _add38r = _sin34r + _mul37r; _add38i = _sin34i + _mul37i;
    { int _idx = 6; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add38r; cIm[_idx] = _add38i; } }
    double _abs39r = 0, _abs39i = 0;
    _abs39r = c_abs(x1r, x1i); _abs39i = 0;
    double _c40r = 0, _c40i = 0;
    _c40r = 1.0; _c40i = 0;
    double _add41r = 0, _add41i = 0;
    _add41r = _abs39r + _c40r; _add41i = _abs39i + _c40i;
    double _log42r = 0, _log42i = 0;
    c_log(_add41r, _add41i, &_log42r, &_log42i);
    double _re43r = 0, _re43i = 0;
    _re43r = x2r; _re43i = 0;
    double _c44r = 0, _c44i = 0;
    _c44r = 3.0; _c44i = 0;
    double _pow45r = 0, _pow45i = 0;
    c_mul(_re43r, _re43i, _re43r, _re43i, &_pow45r, &_pow45i);
    c_mul(_pow45r, _pow45i, _re43r, _re43i, &_pow45r, &_pow45i);
    double _add46r = 0, _add46i = 0;
    _add46r = _log42r + _pow45r; _add46i = _log42i + _pow45i;
    double _im47r = 0, _im47i = 0;
    _im47r = x1i; _im47i = 0;
    double _im48r = 0, _im48i = 0;
    _im48r = x2i; _im48i = 0;
    double _mul49r = 0, _mul49i = 0;
    c_mul(_im47r, _im47i, _im48r, _im48i, &_mul49r, &_mul49i);
    double _sub50r = 0, _sub50i = 0;
    _sub50r = _add46r - _mul49r; _sub50i = _add46i - _mul49i;
    { int _idx = 9; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub50r; cIm[_idx] = _sub50i; } }
    double _c51r = 0, _c51i = 0;
    _c51r = 2.0; _c51i = 0;
    double _pow52r = 0, _pow52i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow52r, &_pow52i);
    double _c53r = 0, _c53i = 0;
    _c53r = 2.0; _c53i = 0;
    double _pow54r = 0, _pow54i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow54r, &_pow54i);
    double _add55r = 0, _add55i = 0;
    _add55r = _pow52r + _pow54r; _add55i = _pow52i + _pow54i;
    double _sin56r = 0, _sin56i = 0;
    c_sin(x1r, x1i, &_sin56r, &_sin56i);
    double _mul57r = 0, _mul57i = 0;
    c_mul(_add55r, _add55i, _sin56r, _sin56i, &_mul57r, &_mul57i);
    double _cos58r = 0, _cos58i = 0;
    c_cos(x2r, x2i, &_cos58r, &_cos58i);
    double _sub59r = 0, _sub59i = 0;
    _sub59r = _mul57r - _cos58r; _sub59i = _mul57i - _cos58i;
    { int _idx = 12; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub59r; cIm[_idx] = _sub59i; } }
    double _re60r = 0, _re60i = 0;
    _re60r = x1r; _re60i = 0;
    double _re61r = 0, _re61i = 0;
    _re61r = x2r; _re61i = 0;
    double _mul62r = 0, _mul62i = 0;
    c_mul(_re60r, _re60i, _re61r, _re61i, &_mul62r, &_mul62i);
    double _im63r = 0, _im63i = 0;
    _im63r = x1i; _im63i = 0;
    double _im64r = 0, _im64i = 0;
    _im64r = x2i; _im64i = 0;
    double _mul65r = 0, _mul65i = 0;
    c_mul(_im63r, _im63i, _im64r, _im64i, &_mul65r, &_mul65i);
    double _add66r = 0, _add66i = 0;
    _add66r = _mul62r + _mul65r; _add66i = _mul62i + _mul65i;
    double _mul67r = 0, _mul67i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul67r, &_mul67i);
    double _ang68r = 0, _ang68i = 0;
    _ang68r = c_arg(_mul67r, _mul67i); _ang68i = 0;
    double _add69r = 0, _add69i = 0;
    _add69r = _add66r + _ang68r; _add69i = _add66i + _ang68i;
    { int _idx = 15; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add69r; cIm[_idx] = _add69i; } }
    double _add70r = 0, _add70i = 0;
    _add70r = x1r + x2r; _add70i = x1i + x2i;
    double _abs71r = 0, _abs71i = 0;
    _abs71r = c_abs(_add70r, _add70i); _abs71i = 0;
    double _ang72r = 0, _ang72i = 0;
    _ang72r = c_arg(x1r, x1i); _ang72i = 0;
    double _sin73r = 0, _sin73i = 0;
    c_sin(_ang72r, _ang72i, &_sin73r, &_sin73i);
    double _mul74r = 0, _mul74i = 0;
    c_mul(_abs71r, _abs71i, _sin73r, _sin73i, &_mul74r, &_mul74i);
    double _abs75r = 0, _abs75i = 0;
    _abs75r = c_abs(x2r, x2i); _abs75i = 0;
    double _cos76r = 0, _cos76i = 0;
    c_cos(_abs75r, _abs75i, &_cos76r, &_cos76i);
    double _sub77r = 0, _sub77i = 0;
    _sub77r = _mul74r - _cos76r; _sub77i = _mul74i - _cos76i;
    { int _idx = 18; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub77r; cIm[_idx] = _sub77i; } }
    double _c78r = 0, _c78i = 0;
    _c78r = 3.0; _c78i = 0;
    double _pow79r = 0, _pow79i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow79r, &_pow79i);
    c_mul(_pow79r, _pow79i, x1r, x1i, &_pow79r, &_pow79i);
    double _conj80r = 0, _conj80i = 0;
    _conj80r = _pow79r; _conj80i = -(_pow79i);
    double _c81r = 0, _c81i = 0;
    _c81r = 3.0; _c81i = 0;
    double _pow82r = 0, _pow82i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow82r, &_pow82i);
    c_mul(_pow82r, _pow82i, x2r, x2i, &_pow82r, &_pow82i);
    double _add83r = 0, _add83i = 0;
    _add83r = _conj80r + _pow82r; _add83i = _conj80i + _pow82i;
    double _mul84r = 0, _mul84i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul84r, &_mul84i);
    double _abs85r = 0, _abs85i = 0;
    _abs85r = c_abs(_mul84r, _mul84i); _abs85i = 0;
    double _c86r = 0, _c86i = 0;
    _c86r = 1.0; _c86i = 0;
    double _add87r = 0, _add87i = 0;
    _add87r = _abs85r + _c86r; _add87i = _abs85i + _c86i;
    double _log88r = 0, _log88i = 0;
    c_log(_add87r, _add87i, &_log88r, &_log88i);
    double _sub89r = 0, _sub89i = 0;
    _sub89r = _add83r - _log88r; _sub89i = _add83i - _log88i;
    { int _idx = 21; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub89r; cIm[_idx] = _sub89i; } }
    double _c90r = 0, _c90i = 0;
    _c90r = 2.0; _c90i = 0;
    double _pow91r = 0, _pow91i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow91r, &_pow91i);
    double _sin92r = 0, _sin92i = 0;
    c_sin(_pow91r, _pow91i, &_sin92r, &_sin92i);
    double _c93r = 0, _c93i = 0;
    _c93r = 2.0; _c93i = 0;
    double _pow94r = 0, _pow94i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow94r, &_pow94i);
    double _cos95r = 0, _cos95i = 0;
    c_cos(_pow94r, _pow94i, &_cos95r, &_cos95i);
    double _add96r = 0, _add96i = 0;
    _add96r = _sin92r + _cos95r; _add96i = _sin92i + _cos95i;
    double _mul97r = 0, _mul97i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul97r, &_mul97i);
    double _re98r = 0, _re98i = 0;
    _re98r = _mul97r; _re98i = 0;
    double _sub99r = 0, _sub99i = 0;
    _sub99r = _add96r - _re98r; _sub99i = _add96i - _re98i;
    { int _idx = 24; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub99r; cIm[_idx] = _sub99i; } }
    double _c100r = 0, _c100i = 0;
    _c100r = 2.0; _c100i = 0;
    double _pow101r = 0, _pow101i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow101r, &_pow101i);
    double _im102r = 0, _im102i = 0;
    _im102r = _pow101i; _im102i = 0;
    double _c103r = 0, _c103i = 0;
    _c103r = 2.0; _c103i = 0;
    double _pow104r = 0, _pow104i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow104r, &_pow104i);
    double _re105r = 0, _re105i = 0;
    _re105r = _pow104r; _re105i = 0;
    double _sub106r = 0, _sub106i = 0;
    _sub106r = _im102r - _re105r; _sub106i = _im102i - _re105i;
    double _mul107r = 0, _mul107i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul107r, &_mul107i);
    double _re108r = 0, _re108i = 0;
    _re108r = _mul107r; _re108i = 0;
    double _sub109r = 0, _sub109i = 0;
    _sub109r = _sub106r - _re108r; _sub109i = _sub106i - _re108i;
    { int _idx = 27; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub109r; cIm[_idx] = _sub109i; } }
    double _abs110r = 0, _abs110i = 0;
    _abs110r = c_abs(x1r, x1i); _abs110i = 0;
    double _c111r = 0, _c111i = 0;
    _c111r = 2.0; _c111i = 0;
    double _pow112r = 0, _pow112i = 0;
    c_mul(_abs110r, _abs110i, _abs110r, _abs110i, &_pow112r, &_pow112i);
    double _cos113r = 0, _cos113i = 0;
    c_cos(x2r, x2i, &_cos113r, &_cos113i);
    double _mul114r = 0, _mul114i = 0;
    c_mul(_pow112r, _pow112i, _cos113r, _cos113i, &_mul114r, &_mul114i);
    double _abs115r = 0, _abs115i = 0;
    _abs115r = c_abs(x2r, x2i); _abs115i = 0;
    double _sin116r = 0, _sin116i = 0;
    c_sin(_abs115r, _abs115i, &_sin116r, &_sin116i);
    double _sub117r = 0, _sub117i = 0;
    _sub117r = _mul114r - _sin116r; _sub117i = _mul114i - _sin116i;
    { int _idx = 30; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub117r; cIm[_idx] = _sub117i; } }
    double _c118r = 0, _c118i = 0;
    _c118r = 3.0; _c118i = 0;
    double _pow119r = 0, _pow119i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow119r, &_pow119i);
    c_mul(_pow119r, _pow119i, x1r, x1i, &_pow119r, &_pow119i);
    double _re120r = 0, _re120i = 0;
    _re120r = _pow119r; _re120i = 0;
    double _c121r = 0, _c121i = 0;
    _c121r = 3.0; _c121i = 0;
    double _pow122r = 0, _pow122i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow122r, &_pow122i);
    c_mul(_pow122r, _pow122i, x2r, x2i, &_pow122r, &_pow122i);
    double _im123r = 0, _im123i = 0;
    _im123r = _pow122i; _im123i = 0;
    double _sub124r = 0, _sub124i = 0;
    _sub124r = _re120r - _im123r; _sub124i = _re120i - _im123i;
    double _add125r = 0, _add125i = 0;
    _add125r = x1r + x2r; _add125i = x1i + x2i;
    double _abs126r = 0, _abs126i = 0;
    _abs126r = c_abs(_add125r, _add125i); _abs126i = 0;
    double _c127r = 0, _c127i = 0;
    _c127r = 1.0; _c127i = 0;
    double _add128r = 0, _add128i = 0;
    _add128r = _abs126r + _c127r; _add128i = _abs126i + _c127i;
    double _log129r = 0, _log129i = 0;
    c_log(_add128r, _add128i, &_log129r, &_log129i);
    double _add130r = 0, _add130i = 0;
    _add130r = _sub124r + _log129r; _add130i = _sub124i + _log129i;
    { int _idx = 33; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add130r; cIm[_idx] = _add130i; } }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_269_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 36; j++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 3.0; _c1i = 0;
        double _mod2r = 0, _mod2i = 0;
        _mod2r = fmod(j, _c1r); _mod2i = 0;
        double _c3r = 0, _c3i = 0;
        _c3r = 1.0; _c3i = 0;
        if (_mod2r == _c3r) {
            double _re4r = 0, _re4i = 0;
            _re4r = x1r; _re4i = 0;
            double _pow5r = 0, _pow5i = 0;
            c_powr(_re4r, _re4i, j, &_pow5r, &_pow5i);
            double _im6r = 0, _im6i = 0;
            _im6r = x2i; _im6i = 0;
            double _c7r = 0, _c7i = 0;
            _c7r = 5.0; _c7i = 0;
            double _mod8r = 0, _mod8i = 0;
            _mod8r = fmod(j, _c7r); _mod8i = 0;
            double _c9r = 0, _c9i = 0;
            _c9r = 1.0; _c9i = 0;
            double _add10r = 0, _add10i = 0;
            _add10r = _mod8r + _c9r; _add10i = _mod8i + _c9i;
            double _pow11r = 0, _pow11i = 0;
            c_powc(_im6r, _im6i, _add10r, _add10i, &_pow11r, &_pow11i);
            double _add12r = 0, _add12i = 0;
            _add12r = _pow5r + _pow11r; _add12i = _pow5i + _pow11i;
            double _ang13r = 0, _ang13i = 0;
            _ang13r = c_arg(x1r, x1i); _ang13i = 0;
            double _mul14r = 0, _mul14i = 0;
            c_mul(j, 0, _ang13r, _ang13i, &_mul14r, &_mul14i);
            double _sin15r = 0, _sin15i = 0;
            c_sin(_mul14r, _mul14i, &_sin15r, &_sin15i);
            double _mul16r = 0, _mul16i = 0;
            c_mul(_add12r, _add12i, _sin15r, _sin15i, &_mul16r, &_mul16i);
            double _ang17r = 0, _ang17i = 0;
            _ang17r = c_arg(x2r, x2i); _ang17i = 0;
            double _mul18r = 0, _mul18i = 0;
            c_mul(j, 0, _ang17r, _ang17i, &_mul18r, &_mul18i);
            double _cos19r = 0, _cos19i = 0;
            c_cos(_mul18r, _mul18i, &_cos19r, &_cos19i);
            double _add20r = 0, _add20i = 0;
            _add20r = _mul16r + _cos19r; _add20i = _mul16i + _cos19i;
            { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add20r; cIm[_idx] = _add20i; } }
        } else {
            double _c21r = 0, _c21i = 0;
            _c21r = 3.0; _c21i = 0;
            double _mod22r = 0, _mod22i = 0;
            _mod22r = fmod(j, _c21r); _mod22i = 0;
            double _c23r = 0, _c23i = 0;
            _c23r = 2.0; _c23i = 0;
            if (_mod22r == _c23r) {
                double _abs24r = 0, _abs24i = 0;
                _abs24r = c_abs(x1r, x1i); _abs24i = 0;
                double _abs25r = 0, _abs25i = 0;
                _abs25r = c_abs(x2r, x2i); _abs25i = 0;
                double _mul26r = 0, _mul26i = 0;
                c_mul(_abs24r, _abs24i, _abs25r, _abs25i, &_mul26r, &_mul26i);
                double _c27r = 0, _c27i = 0;
                _c27r = 1.0; _c27i = 0;
                double _add28r = 0, _add28i = 0;
                _add28r = j + _c27r; _add28i = 0 + _c27i;
                double _c29r = 0, _c29i = 0;
                _c29r = 7.0; _c29i = 0;
                double _div30r = 0, _div30i = 0;
                c_div(_add28r, _add28i, _c29r, _c29i, &_div30r, &_div30i);
                double _pow31r = 0, _pow31i = 0;
                c_powc(_mul26r, _mul26i, _div30r, _div30i, &_pow31r, &_pow31i);
                double _abs32r = 0, _abs32i = 0;
                _abs32r = c_abs(x1r, x1i); _abs32i = 0;
                double _c33r = 0, _c33i = 0;
                _c33r = 1.0; _c33i = 0;
                double _add34r = 0, _add34i = 0;
                _add34r = _abs32r + _c33r; _add34i = _abs32i + _c33i;
                double _log35r = 0, _log35i = 0;
                c_log(_add34r, _add34i, &_log35r, &_log35i);
                double _abs36r = 0, _abs36i = 0;
                _abs36r = c_abs(x2r, x2i); _abs36i = 0;
                double _c37r = 0, _c37i = 0;
                _c37r = 1.0; _c37i = 0;
                double _add38r = 0, _add38i = 0;
                _add38r = _abs36r + _c37r; _add38i = _abs36i + _c37i;
                double _log39r = 0, _log39i = 0;
                c_log(_add38r, _add38i, &_log39r, &_log39i);
                double _mul40r = 0, _mul40i = 0;
                c_mul(_log35r, _log35i, _log39r, _log39i, &_mul40r, &_mul40i);
                double _add41r = 0, _add41i = 0;
                _add41r = _pow31r + _mul40r; _add41i = _pow31i + _mul40i;
                { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add41r; cIm[_idx] = _add41i; } }
            } else {
                double _conj42r = 0, _conj42i = 0;
                _conj42r = x1r; _conj42i = -(x1i);
                double _c43r = 0, _c43i = 0;
                _c43r = 4.0; _c43i = 0;
                double _mod44r = 0, _mod44i = 0;
                _mod44r = fmod(j, _c43r); _mod44i = 0;
                double _pow45r = 0, _pow45i = 0;
                c_powc(x2r, x2i, _mod44r, _mod44i, &_pow45r, &_pow45i);
                double _mul46r = 0, _mul46i = 0;
                c_mul(_conj42r, _conj42i, _pow45r, _pow45i, &_mul46r, &_mul46i);
                double _conj47r = 0, _conj47i = 0;
                _conj47r = x2r; _conj47i = -(x2i);
                double _c48r = 0, _c48i = 0;
                _c48r = 3.0; _c48i = 0;
                double _mod49r = 0, _mod49i = 0;
                _mod49r = fmod(j, _c48r); _mod49i = 0;
                double _pow50r = 0, _pow50i = 0;
                c_powc(x1r, x1i, _mod49r, _mod49i, &_pow50r, &_pow50i);
                double _mul51r = 0, _mul51i = 0;
                c_mul(_conj47r, _conj47i, _pow50r, _pow50i, &_mul51r, &_mul51i);
                double _sub52r = 0, _sub52i = 0;
                _sub52r = _mul46r - _mul51r; _sub52i = _mul46i - _mul51i;
                { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub52r; cIm[_idx] = _sub52i; } }
            }
        }
    }
    double _re53r = 0, _re53i = 0;
    _re53r = x1r; _re53i = 0;
    double _im54r = 0, _im54i = 0;
    _im54r = x2i; _im54i = 0;
    double _sum55r = 0, _sum55i = 0;
    _sum55r = _re53r + _im54r; _sum55i = _re53i + _im54i;
    double _abs56r = 0, _abs56i = 0;
    _abs56r = c_abs(x1r, x1i); _abs56i = 0;
    double _abs57r = 0, _abs57i = 0;
    _abs57r = c_abs(x2r, x2i); _abs57i = 0;
    double _prod58r = 0, _prod58i = 0;
    c_mul(_abs56r, _abs56i, _abs57r, _abs57i, &_prod58r, &_prod58i);
    double _add59r = 0, _add59i = 0;
    _add59r = _sum55r + _prod58r; _add59i = _sum55i + _prod58i;
    { int _idx = 3; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add59r; cIm[_idx] = _add59i; } }
    double _mul60r = 0, _mul60i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul60r, &_mul60i);
    double _sin61r = 0, _sin61i = 0;
    c_sin(_mul60r, _mul60i, &_sin61r, &_sin61i);
    double _sub62r = 0, _sub62i = 0;
    _sub62r = x1r - x2r; _sub62i = x1i - x2i;
    double _cos63r = 0, _cos63i = 0;
    c_cos(_sub62r, _sub62i, &_cos63r, &_cos63i);
    double _add64r = 0, _add64i = 0;
    _add64r = _sin61r + _cos63r; _add64i = _sin61i + _cos63i;
    double _add65r = 0, _add65i = 0;
    _add65r = x1r + x2r; _add65i = x1i + x2i;
    double _abs66r = 0, _abs66i = 0;
    _abs66r = c_abs(_add65r, _add65i); _abs66i = 0;
    double _c67r = 0, _c67i = 0;
    _c67r = 1.0; _c67i = 0;
    double _add68r = 0, _add68i = 0;
    _add68r = _abs66r + _c67r; _add68i = _abs66i + _c67i;
    double _log69r = 0, _log69i = 0;
    c_log(_add68r, _add68i, &_log69r, &_log69i);
    double _add70r = 0, _add70i = 0;
    _add70r = _add64r + _log69r; _add70i = _add64i + _log69i;
    { int _idx = 9; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add70r; cIm[_idx] = _add70i; } }
    double _re71r = 0, _re71i = 0;
    _re71r = x1r; _re71i = 0;
    double _c72r = 0, _c72i = 0;
    _c72r = 2.0; _c72i = 0;
    double _pow73r = 0, _pow73i = 0;
    c_mul(_re71r, _re71i, _re71r, _re71i, &_pow73r, &_pow73i);
    double _im74r = 0, _im74i = 0;
    _im74r = x1i; _im74i = 0;
    double _c75r = 0, _c75i = 0;
    _c75r = 2.0; _c75i = 0;
    double _pow76r = 0, _pow76i = 0;
    c_mul(_im74r, _im74i, _im74r, _im74i, &_pow76r, &_pow76i);
    double _sub77r = 0, _sub77i = 0;
    _sub77r = _pow73r - _pow76r; _sub77i = _pow73i - _pow76i;
    double _re78r = 0, _re78i = 0;
    _re78r = x2r; _re78i = 0;
    double _c79r = 0, _c79i = 0;
    _c79r = 2.0; _c79i = 0;
    double _pow80r = 0, _pow80i = 0;
    c_mul(_re78r, _re78i, _re78r, _re78i, &_pow80r, &_pow80i);
    double _im81r = 0, _im81i = 0;
    _im81r = x2i; _im81i = 0;
    double _c82r = 0, _c82i = 0;
    _c82r = 2.0; _c82i = 0;
    double _pow83r = 0, _pow83i = 0;
    c_mul(_im81r, _im81i, _im81r, _im81i, &_pow83r, &_pow83i);
    double _sub84r = 0, _sub84i = 0;
    _sub84r = _pow80r - _pow83r; _sub84i = _pow80i - _pow83i;
    double _add85r = 0, _add85i = 0;
    _add85r = _sub77r + _sub84r; _add85i = _sub77i + _sub84i;
    { int _idx = 15; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add85r; cIm[_idx] = _add85i; } }
    double _mul86r = 0, _mul86i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul86r, &_mul86i);
    double _abs87r = 0, _abs87i = 0;
    _abs87r = c_abs(_mul86r, _mul86i); _abs87i = 0;
    double _add88r = 0, _add88i = 0;
    _add88r = x1r + x2r; _add88i = x1i + x2i;
    double _ang89r = 0, _ang89i = 0;
    _ang89r = c_arg(_add88r, _add88i); _ang89i = 0;
    double _mul90r = 0, _mul90i = 0;
    c_mul(_abs87r, _abs87i, _ang89r, _ang89i, &_mul90r, &_mul90i);
    double _sub91r = 0, _sub91i = 0;
    _sub91r = x1r - x2r; _sub91i = x1i - x2i;
    double _conj92r = 0, _conj92i = 0;
    _conj92r = _sub91r; _conj92i = -(_sub91i);
    double _add93r = 0, _add93i = 0;
    _add93r = _mul90r + _conj92r; _add93i = _mul90i + _conj92i;
    { int _idx = 21; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add93r; cIm[_idx] = _add93i; } }
    double _re94r = 0, _re94i = 0;
    _re94r = x1r; _re94i = 0;
    double _im95r = 0, _im95i = 0;
    _im95r = x2i; _im95i = 0;
    double _mul96r = 0, _mul96i = 0;
    c_mul(_re94r, _re94i, _im95r, _im95i, &_mul96r, &_mul96i);
    double _sin97r = 0, _sin97i = 0;
    c_sin(_mul96r, _mul96i, &_sin97r, &_sin97i);
    double _im98r = 0, _im98i = 0;
    _im98r = x1i; _im98i = 0;
    double _re99r = 0, _re99i = 0;
    _re99r = x2r; _re99i = 0;
    double _mul100r = 0, _mul100i = 0;
    c_mul(_im98r, _im98i, _re99r, _re99i, &_mul100r, &_mul100i);
    double _cos101r = 0, _cos101i = 0;
    c_cos(_mul100r, _mul100i, &_cos101r, &_cos101i);
    double _add102r = 0, _add102i = 0;
    _add102r = _sin97r + _cos101r; _add102i = _sin97i + _cos101i;
    { int _idx = 27; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add102r; cIm[_idx] = _add102i; } }
    double _abs103r = 0, _abs103i = 0;
    _abs103r = c_abs(x1r, x1i); _abs103i = 0;
    double _c104r = 0, _c104i = 0;
    _c104r = 3.0; _c104i = 0;
    double _pow105r = 0, _pow105i = 0;
    c_mul(_abs103r, _abs103i, _abs103r, _abs103i, &_pow105r, &_pow105i);
    c_mul(_pow105r, _pow105i, _abs103r, _abs103i, &_pow105r, &_pow105i);
    double _abs106r = 0, _abs106i = 0;
    _abs106r = c_abs(x2r, x2i); _abs106i = 0;
    double _c107r = 0, _c107i = 0;
    _c107r = 3.0; _c107i = 0;
    double _pow108r = 0, _pow108i = 0;
    c_mul(_abs106r, _abs106i, _abs106r, _abs106i, &_pow108r, &_pow108i);
    c_mul(_pow108r, _pow108i, _abs106r, _abs106i, &_pow108r, &_pow108i);
    double _add109r = 0, _add109i = 0;
    _add109r = _pow105r + _pow108r; _add109i = _pow105i + _pow108i;
    double _c110r = 0, _c110i = 0;
    _c110r = 1.0; _c110i = 0;
    double _add111r = 0, _add111i = 0;
    _add111r = _add109r + _c110r; _add111i = _add109i + _c110i;
    double _log112r = 0, _log112i = 0;
    c_log(_add111r, _add111i, &_log112r, &_log112i);
    double _conj113r = 0, _conj113i = 0;
    _conj113r = x2r; _conj113i = -(x2i);
    double _mul114r = 0, _mul114i = 0;
    c_mul(x1r, x1i, _conj113r, _conj113i, &_mul114r, &_mul114i);
    double _re115r = 0, _re115i = 0;
    _re115r = _mul114r; _re115i = 0;
    double _add116r = 0, _add116i = 0;
    _add116r = _log112r + _re115r; _add116i = _log112i + _re115i;
    { int _idx = 33; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add116r; cIm[_idx] = _add116i; } }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_270_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 36; j++) {
        double _ang1r = 0, _ang1i = 0;
        _ang1r = c_arg(x1r, x1i); _ang1i = 0;
        double _mul2r = 0, _mul2i = 0;
        c_mul(_ang1r, _ang1i, j, 0, &_mul2r, &_mul2i);
        double _ang3r = 0, _ang3i = 0;
        _ang3r = c_arg(x2r, x2i); _ang3i = 0;
        double _mul4r = 0, _mul4i = 0;
        c_mul(j, 0, _ang3r, _ang3i, &_mul4r, &_mul4i);
        double _sin5r = 0, _sin5i = 0;
        c_sin(_mul4r, _mul4i, &_sin5r, &_sin5i);
        double _add6r = 0, _add6i = 0;
        _add6r = _mul2r + _sin5r; _add6i = _mul2i + _sin5i;
        double phase1_r = _add6r, phase1_i = _add6i;
        double _abs7r = 0, _abs7i = 0;
        _abs7r = c_abs(x1r, x1i); _abs7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _abs7r + j; _add8i = _abs7i + 0;
        double _log9r = 0, _log9i = 0;
        c_log(_add8r, _add8i, &_log9r, &_log9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(j, 0, M_PI, 0, &_mul10r, &_mul10i);
        double _c11r = 0, _c11i = 0;
        _c11r = 7.0; _c11i = 0;
        double _div12r = 0, _div12i = 0;
        c_div(_mul10r, _mul10i, _c11r, _c11i, &_div12r, &_div12i);
        double _cos13r = 0, _cos13i = 0;
        c_cos(_div12r, _div12i, &_cos13r, &_cos13i);
        double _mul14r = 0, _mul14i = 0;
        c_mul(_log9r, _log9i, _cos13r, _cos13i, &_mul14r, &_mul14i);
        double magnitude1_r = _mul14r, magnitude1_i = _mul14i;
        double _c15r = 0, _c15i = 0;
        _c15r = 0.0; _c15i = 1.0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(_c15r, _c15i, phase1_r, phase1_i, &_mul16r, &_mul16i);
        double _exp17r = 0, _exp17i = 0;
        c_exp2(_mul16r, _mul16i, &_exp17r, &_exp17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(magnitude1_r, magnitude1_i, _exp17r, _exp17i, &_mul18r, &_mul18i);
        double term1_r = _mul18r, term1_i = _mul18i;
        double _ang19r = 0, _ang19i = 0;
        _ang19r = c_arg(x2r, x2i); _ang19i = 0;
        double _c20r = 0, _c20i = 0;
        _c20r = 35.0; _c20i = 0;
        double _sub21r = 0, _sub21i = 0;
        _sub21r = _c20r - j; _sub21i = _c20i - 0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(_ang19r, _ang19i, _sub21r, _sub21i, &_mul22r, &_mul22i);
        double _ang23r = 0, _ang23i = 0;
        _ang23r = c_arg(x1r, x1i); _ang23i = 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(j, 0, _ang23r, _ang23i, &_mul24r, &_mul24i);
        double _cos25r = 0, _cos25i = 0;
        c_cos(_mul24r, _mul24i, &_cos25r, &_cos25i);
        double _add26r = 0, _add26i = 0;
        _add26r = _mul22r + _cos25r; _add26i = _mul22i + _cos25i;
        double phase2_r = _add26r, phase2_i = _add26i;
        double _abs27r = 0, _abs27i = 0;
        _abs27r = c_abs(x2r, x2i); _abs27i = 0;
        double _c28r = 0, _c28i = 0;
        _c28r = 35.0; _c28i = 0;
        double _sub29r = 0, _sub29i = 0;
        _sub29r = _c28r - j; _sub29i = _c28i - 0;
        double _add30r = 0, _add30i = 0;
        _add30r = _abs27r + _sub29r; _add30i = _abs27i + _sub29i;
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
        double magnitude2_r = _mul36r, magnitude2_i = _mul36i;
        double _c37r = 0, _c37i = 0;
        _c37r = 0.0; _c37i = 1.0;
        double _mul38r = 0, _mul38i = 0;
        c_mul(_c37r, _c37i, phase2_r, phase2_i, &_mul38r, &_mul38i);
        double _exp39r = 0, _exp39i = 0;
        c_exp2(_mul38r, _mul38i, &_exp39r, &_exp39i);
        double _mul40r = 0, _mul40i = 0;
        c_mul(magnitude2_r, magnitude2_i, _exp39r, _exp39i, &_mul40r, &_mul40i);
        double term2_r = _mul40r, term2_i = _mul40i;
        double _add41r = 0, _add41i = 0;
        _add41r = term1_r + term2_r; _add41i = term1_i + term2_i;
        double _conj42r = 0, _conj42i = 0;
        _conj42r = x1r; _conj42i = -(x1i);
        double _c43r = 0, _c43i = 0;
        _c43r = 5.0; _c43i = 0;
        double _mod44r = 0, _mod44i = 0;
        _mod44r = fmod(j, _c43r); _mod44i = 0;
        double _pow45r = 0, _pow45i = 0;
        c_powc(_conj42r, _conj42i, _mod44r, _mod44i, &_pow45r, &_pow45i);
        double _conj46r = 0, _conj46i = 0;
        _conj46r = x2r; _conj46i = -(x2i);
        double _c47r = 0, _c47i = 0;
        _c47r = 3.0; _c47i = 0;
        double _mod48r = 0, _mod48i = 0;
        _mod48r = fmod(j, _c47r); _mod48i = 0;
        double _pow49r = 0, _pow49i = 0;
        c_powc(_conj46r, _conj46i, _mod48r, _mod48i, &_pow49r, &_pow49i);
        double _mul50r = 0, _mul50i = 0;
        c_mul(_pow45r, _pow45i, _pow49r, _pow49i, &_mul50r, &_mul50i);
        double _add51r = 0, _add51i = 0;
        _add51r = _add41r + _mul50r; _add51i = _add41i + _mul50i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add51r; cIm[_idx] = _add51i; } }
    }
    for (int k = 2; k < 35; k += 3) {
        double _mul52r = 0, _mul52i = 0;
        c_mul(x1r, x1i, k, 0, &_mul52r, &_mul52i);
        double _abs53r = 0, _abs53i = 0;
        _abs53r = c_abs(_mul52r, _mul52i); _abs53i = 0;
        double _sin54r = 0, _sin54i = 0;
        c_sin(_abs53r, _abs53i, &_sin54r, &_sin54i);
        double _add55r = 0, _add55i = 0;
        _add55r = x2r + k; _add55i = x2i + 0;
        double _abs56r = 0, _abs56i = 0;
        _abs56r = c_abs(_add55r, _add55i); _abs56i = 0;
        double _cos57r = 0, _cos57i = 0;
        c_cos(_abs56r, _abs56i, &_cos57r, &_cos57i);
        double _add58r = 0, _add58i = 0;
        _add58r = _sin54r + _cos57r; _add58i = _sin54i + _cos57i;
        { double _tr = cRe[(k - 1)]*_add58r - cIm[(k - 1)]*_add58i; cIm[(k - 1)] = cRe[(k - 1)]*_add58i + cIm[(k - 1)]*_add58r; cRe[(k - 1)] = _tr; }
    }
    for (int r = 1; r < 36; r += 5) {
        double _c59r = 0, _c59i = 0;
        _c59r = 0.0; _c59i = 1.0;
        double _add60r = 0, _add60i = 0;
        _add60r = x1r + r; _add60i = x1i + 0;
        double _abs61r = 0, _abs61i = 0;
        _abs61r = c_abs(_add60r, _add60i); _abs61i = 0;
        double _c62r = 0, _c62i = 0;
        _c62r = 1.0; _c62i = 0;
        double _add63r = 0, _add63i = 0;
        _add63r = _abs61r + _c62r; _add63i = _abs61i + _c62i;
        double _log64r = 0, _log64i = 0;
        c_log(_add63r, _add63i, &_log64r, &_log64i);
        double _mul65r = 0, _mul65i = 0;
        c_mul(_c59r, _c59i, _log64r, _log64i, &_mul65r, &_mul65i);
        double _ang66r = 0, _ang66i = 0;
        _ang66r = c_arg(x2r, x2i); _ang66i = 0;
        double _mul67r = 0, _mul67i = 0;
        c_mul(_ang66r, _ang66i, r, 0, &_mul67r, &_mul67i);
        double _sin68r = 0, _sin68i = 0;
        c_sin(_mul67r, _mul67i, &_sin68r, &_sin68i);
        double _mul69r = 0, _mul69i = 0;
        c_mul(_mul65r, _mul65i, _sin68r, _sin68i, &_mul69r, &_mul69i);
        cRe[(r - 1)] += _mul69r; cIm[(r - 1)] += _mul69i;
    }
    double _re70r = 0, _re70i = 0;
    _re70r = x1r; _re70i = 0;
    double _re71r = 0, _re71i = 0;
    _re71r = x2r; _re71i = 0;
    double _add72r = 0, _add72i = 0;
    _add72r = _re70r + _re71r; _add72i = _re70i + _re71i;
    { int _idx = 0; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add72r; cIm[_idx] = _add72i; } }
    double _im73r = 0, _im73i = 0;
    _im73r = x1i; _im73i = 0;
    double _im74r = 0, _im74i = 0;
    _im74r = x2i; _im74i = 0;
    double _sub75r = 0, _sub75i = 0;
    _sub75r = _im73r - _im74r; _sub75i = _im73i - _im74i;
    double _mul76r = 0, _mul76i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul76r, &_mul76i);
    double _conj77r = 0, _conj77i = 0;
    _conj77r = _mul76r; _conj77i = -(_mul76i);
    double _add78r = 0, _add78i = 0;
    _add78r = _sub75r + _conj77r; _add78i = _sub75i + _conj77i;
    { int _idx = 34; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add78r; cIm[_idx] = _add78i; } }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_271_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 36; j++) {
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
        double _abs8r = 0, _abs8i = 0;
        _abs8r = c_abs(x2r, x2i); _abs8i = 0;
        double _mul9r = 0, _mul9i = 0;
        c_mul(_abs8r, _abs8i, j, 0, &_mul9r, &_mul9i);
        double _cos10r = 0, _cos10i = 0;
        c_cos(_mul9r, _mul9i, &_cos10r, &_cos10i);
        double _add11r = 0, _add11i = 0;
        _add11r = _mul7r + _cos10r; _add11i = _mul7i + _cos10i;
        double mag_r = _add11r, mag_i = _add11i;
        double _re12r = 0, _re12i = 0;
        _re12r = x1r; _re12i = 0;
        double _mul13r = 0, _mul13i = 0;
        c_mul(_re12r, _re12i, j, 0, &_mul13r, &_mul13i);
        double _im14r = 0, _im14i = 0;
        _im14r = x2i; _im14i = 0;
        double _c15r = 0, _c15i = 0;
        _c15r = 1.0; _c15i = 0;
        double _add16r = 0, _add16i = 0;
        _add16r = j + _c15r; _add16i = 0 + _c15i;
        double _div17r = 0, _div17i = 0;
        c_div(_im14r, _im14i, _add16r, _add16i, &_div17r, &_div17i);
        double _add18r = 0, _add18i = 0;
        _add18r = _mul13r + _div17r; _add18i = _mul13i + _div17i;
        double angle_r = _add18r, angle_i = _add18i;
        double _cos19r = 0, _cos19i = 0;
        c_cos(angle_r, angle_i, &_cos19r, &_cos19i);
        double _sin20r = 0, _sin20i = 0;
        c_sin(angle_r, angle_i, &_sin20r, &_sin20i);
        double _c21r = 0, _c21i = 0;
        _c21r = 0.0; _c21i = 1.0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(_sin20r, _sin20i, _c21r, _c21i, &_mul22r, &_mul22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _cos19r + _mul22r; _add23i = _cos19i + _mul22i;
        double _mul24r = 0, _mul24i = 0;
        c_mul(mag_r, mag_i, _add23r, _add23i, &_mul24r, &_mul24i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul24r; cIm[_idx] = _mul24i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_272_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 1; k < 36; k++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 3.0; _c1i = 0;
        double _add2r = 0, _add2i = 0;
        _add2r = k + _c1r; _add2i = 0 + _c1i;
        double _c3r = 0, _c3i = 0;
        _c3r = 6.0; _c3i = 0;
        double _mod4r = 0, _mod4i = 0;
        _mod4r = fmod(_add2r, _c3r); _mod4i = 0;
        double _c5r = 0, _c5i = 0;
        _c5r = 1.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = _mod4r + _c5r; _add6i = _mod4i + _c5i;
        double j_r = _add6r, j_i = _add6i;
        double _c7r = 0, _c7i = 0;
        _c7r = 4.0; _c7i = 0;
        double _fdiv8r = 0, _fdiv8i = 0;
        c_div(k, 0, _c7r, _c7i, &_fdiv8r, &_fdiv8i);
        _fdiv8r = floor(_fdiv8r); _fdiv8i = 0;
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = _fdiv8r + _c9r; _add10i = _fdiv8i + _c9i;
        double r_r = _add10r, r_i = _add10i;
        double _abs11r = 0, _abs11i = 0;
        _abs11r = c_abs(x1r, x1i); _abs11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = _abs11r + k; _add12i = _abs11i + 0;
        double _log13r = 0, _log13i = 0;
        c_log(_add12r, _add12i, &_log13r, &_log13i);
        double _ang14r = 0, _ang14i = 0;
        _ang14r = c_arg(x2r, x2i); _ang14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(j_r, j_i, _ang14r, _ang14i, &_mul15r, &_mul15i);
        double _sin16r = 0, _sin16i = 0;
        c_sin(_mul15r, _mul15i, &_sin16r, &_sin16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_log13r, _log13i, _sin16r, _sin16i, &_mul17r, &_mul17i);
        double _ang18r = 0, _ang18i = 0;
        _ang18r = c_arg(x1r, x1i); _ang18i = 0;
        double _mul19r = 0, _mul19i = 0;
        c_mul(r_r, r_i, _ang18r, _ang18i, &_mul19r, &_mul19i);
        double _cos20r = 0, _cos20i = 0;
        c_cos(_mul19r, _mul19i, &_cos20r, &_cos20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _mul17r + _cos20r; _add21i = _mul17i + _cos20i;
        double mag_part_r = _add21r, mag_part_i = _add21i;
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x1r, x1i); _ang22i = 0;
        double _pow23r = 0, _pow23i = 0;
        c_powc(_ang22r, _ang22i, j_r, j_i, &_pow23r, &_pow23i);
        double _ang24r = 0, _ang24i = 0;
        _ang24r = c_arg(x2r, x2i); _ang24i = 0;
        double _pow25r = 0, _pow25i = 0;
        c_powc(_ang24r, _ang24i, r_r, r_i, &_pow25r, &_pow25i);
        double _sub26r = 0, _sub26i = 0;
        _sub26r = _pow23r - _pow25r; _sub26i = _pow23i - _pow25i;
        double _sin27r = 0, _sin27i = 0;
        c_sin(k, 0, &_sin27r, &_sin27i);
        double _cos28r = 0, _cos28i = 0;
        c_cos(k, 0, &_cos28r, &_cos28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_sin27r, _sin27i, _cos28r, _cos28i, &_mul29r, &_mul29i);
        double _add30r = 0, _add30i = 0;
        _add30r = _sub26r + _mul29r; _add30i = _sub26i + _mul29i;
        double angle_part_r = _add30r, angle_part_i = _add30i;
        double _cos31r = 0, _cos31i = 0;
        c_cos(angle_part_r, angle_part_i, &_cos31r, &_cos31i);
        double _c32r = 0, _c32i = 0;
        _c32r = 0.0; _c32i = 1.0;
        double _sin33r = 0, _sin33i = 0;
        c_sin(angle_part_r, angle_part_i, &_sin33r, &_sin33i);
        double _mul34r = 0, _mul34i = 0;
        c_mul(_c32r, _c32i, _sin33r, _sin33i, &_mul34r, &_mul34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _cos31r + _mul34r; _add35i = _cos31i + _mul34i;
        double _mul36r = 0, _mul36i = 0;
        c_mul(mag_part_r, mag_part_i, _add35r, _add35i, &_mul36r, &_mul36i);
        double _conj37r = 0, _conj37i = 0;
        _conj37r = x1r; _conj37i = -(x1i);
        double _pow38r = 0, _pow38i = 0;
        c_powc(_conj37r, _conj37i, j_r, j_i, &_pow38r, &_pow38i);
        double _conj39r = 0, _conj39i = 0;
        _conj39r = x2r; _conj39i = -(x2i);
        double _pow40r = 0, _pow40i = 0;
        c_powc(_conj39r, _conj39i, r_r, r_i, &_pow40r, &_pow40i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(_pow38r, _pow38i, _pow40r, _pow40i, &_mul41r, &_mul41i);
        double _add42r = 0, _add42i = 0;
        _add42r = _mul36r + _mul41r; _add42i = _mul36i + _mul41i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add42r; cIm[_idx] = _add42i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_273_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 2.0; _c1i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _c1r; cIm[_idx] = _c1i; } }
    double _c2r = 0, _c2i = 0;
    _c2r = 3.0; _c2i = 0;
    double _neg3r = 0, _neg3i = 0;
    _neg3r = -(_c2r); _neg3i = -(_c2i);
    double _c4r = 0, _c4i = 0;
    _c4r = 0.0; _c4i = 1.0;
    double _add5r = 0, _add5i = 0;
    _add5r = _neg3r + _c4r; _add5i = _neg3i + _c4i;
    { int _idx = 5; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add5r; cIm[_idx] = _add5i; } }
    double _c6r = 0, _c6i = 0;
    _c6r = 4.0; _c6i = 0;
    { int _idx = 11; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _c6r; cIm[_idx] = _c6i; } }
    double _c7r = 0, _c7i = 0;
    _c7r = 0.0; _c7i = 5.0;
    double _neg8r = 0, _neg8i = 0;
    _neg8r = -(_c7r); _neg8i = -(_c7i);
    { int _idx = 17; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _neg8r; cIm[_idx] = _neg8i; } }
    double _c9r = 0, _c9i = 0;
    _c9r = 6.0; _c9i = 0;
    double _c10r = 0, _c10i = 0;
    _c10r = 0.0; _c10i = 2.0;
    double _add11r = 0, _add11i = 0;
    _add11r = _c9r + _c10r; _add11i = _c9i + _c10i;
    { int _idx = 23; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add11r; cIm[_idx] = _add11i; } }
    double _c12r = 0, _c12i = 0;
    _c12r = 7.0; _c12i = 0;
    double _neg13r = 0, _neg13i = 0;
    _neg13r = -(_c12r); _neg13i = -(_c12i);
    { int _idx = 29; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _neg13r; cIm[_idx] = _neg13i; } }
    for (int j = 2; j < 35; j++) {
        double _cf14r = 0, _cf14i = 0;
        { int _idx = j; if (_idx >= 0 && _idx < 35) { _cf14r = cRe[_idx]; _cf14i = cIm[_idx]; } }
        double _c15r = 0, _c15i = 0;
        _c15r = 0.0; _c15i = 0;
        if (_cf14r == _c15r) {
            double _re16r = 0, _re16i = 0;
            _re16r = x1r; _re16i = 0;
            double _pow17r = 0, _pow17i = 0;
            c_powr(_re16r, _re16i, j, &_pow17r, &_pow17i);
            double _im18r = 0, _im18i = 0;
            _im18r = x2i; _im18i = 0;
            double _pow19r = 0, _pow19i = 0;
            c_powr(_im18r, _im18i, j, &_pow19r, &_pow19i);
            double _sub20r = 0, _sub20i = 0;
            _sub20r = _pow17r - _pow19r; _sub20i = _pow17i - _pow19i;
            double _ang21r = 0, _ang21i = 0;
            _ang21r = c_arg(x1r, x1i); _ang21i = 0;
            double _mul22r = 0, _mul22i = 0;
            c_mul(_ang21r, _ang21i, j, 0, &_mul22r, &_mul22i);
            double _abs23r = 0, _abs23i = 0;
            _abs23r = c_abs(x2r, x2i); _abs23i = 0;
            double _add24r = 0, _add24i = 0;
            _add24r = _mul22r + _abs23r; _add24i = _mul22i + _abs23i;
            double _c25r = 0, _c25i = 0;
            _c25r = 0.0; _c25i = 1.0;
            double _mul26r = 0, _mul26i = 0;
            c_mul(_add24r, _add24i, _c25r, _c25i, &_mul26r, &_mul26i);
            double _add27r = 0, _add27i = 0;
            _add27r = _sub20r + _mul26r; _add27i = _sub20i + _mul26i;
            { int _idx = j; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add27r; cIm[_idx] = _add27i; } }
        }
    }
    for (int k = 3; k < 34; k++) {
        double _mul28r = 0, _mul28i = 0;
        c_mul(x1r, x1i, k, 0, &_mul28r, &_mul28i);
        double _sin29r = 0, _sin29i = 0;
        c_sin(_mul28r, _mul28i, &_sin29r, &_sin29i);
        double _div30r = 0, _div30i = 0;
        c_div(x2r, x2i, k, 0, &_div30r, &_div30i);
        double _cos31r = 0, _cos31i = 0;
        c_cos(_div30r, _div30i, &_cos31r, &_cos31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_sin29r, _sin29i, _cos31r, _cos31i, &_mul32r, &_mul32i);
        double _abs33r = 0, _abs33i = 0;
        _abs33r = c_abs(x1r, x1i); _abs33i = 0;
        double _c34r = 0, _c34i = 0;
        _c34r = 1.0; _c34i = 0;
        double _add35r = 0, _add35i = 0;
        _add35r = _abs33r + _c34r; _add35i = _abs33i + _c34i;
        double _log36r = 0, _log36i = 0;
        c_log(_add35r, _add35i, &_log36r, &_log36i);
        double _ang37r = 0, _ang37i = 0;
        _ang37r = c_arg(x2r, x2i); _ang37i = 0;
        double _sin38r = 0, _sin38i = 0;
        c_sin(_ang37r, _ang37i, &_sin38r, &_sin38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(_log36r, _log36i, _sin38r, _sin38i, &_mul39r, &_mul39i);
        double _c40r = 0, _c40i = 0;
        _c40r = 0.0; _c40i = 1.0;
        double _mul41r = 0, _mul41i = 0;
        c_mul(_mul39r, _mul39i, _c40r, _c40i, &_mul41r, &_mul41i);
        double _add42r = 0, _add42i = 0;
        _add42r = _mul32r + _mul41r; _add42i = _mul32i + _mul41i;
        cRe[k] += _add42r; cIm[k] += _add42i;
    }
    double _conj43r = 0, _conj43i = 0;
    _conj43r = x1r; _conj43i = -(x1i);
    double _c44r = 0, _c44i = 0;
    _c44r = 2.0; _c44i = 0;
    double _pow45r = 0, _pow45i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow45r, &_pow45i);
    double _mul46r = 0, _mul46i = 0;
    c_mul(_conj43r, _conj43i, _pow45r, _pow45i, &_mul46r, &_mul46i);
    double _abs47r = 0, _abs47i = 0;
    _abs47r = c_abs(x2r, x2i); _abs47i = 0;
    double _c48r = 0, _c48i = 0;
    _c48r = 0.0; _c48i = 1.0;
    double _mul49r = 0, _mul49i = 0;
    c_mul(_abs47r, _abs47i, _c48r, _c48i, &_mul49r, &_mul49i);
    double _add50r = 0, _add50i = 0;
    _add50r = _mul46r + _mul49r; _add50i = _mul46i + _mul49i;
    { int _idx = 9; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add50r; cIm[_idx] = _add50i; } }
    double _c51r = 0, _c51i = 0;
    _c51r = 3.0; _c51i = 0;
    double _pow52r = 0, _pow52i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow52r, &_pow52i);
    c_mul(_pow52r, _pow52i, x1r, x1i, &_pow52r, &_pow52i);
    double _re53r = 0, _re53i = 0;
    _re53r = _pow52r; _re53i = 0;
    double _c54r = 0, _c54i = 0;
    _c54r = 3.0; _c54i = 0;
    double _pow55r = 0, _pow55i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow55r, &_pow55i);
    c_mul(_pow55r, _pow55i, x2r, x2i, &_pow55r, &_pow55i);
    double _im56r = 0, _im56i = 0;
    _im56r = _pow55i; _im56i = 0;
    double _c57r = 0, _c57i = 0;
    _c57r = 0.0; _c57i = 1.0;
    double _mul58r = 0, _mul58i = 0;
    c_mul(_im56r, _im56i, _c57r, _c57i, &_mul58r, &_mul58i);
    double _add59r = 0, _add59i = 0;
    _add59r = _re53r + _mul58r; _add59i = _re53i + _mul58i;
    { int _idx = 14; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add59r; cIm[_idx] = _add59i; } }
    double _re60r = 0, _re60i = 0;
    _re60r = x1r; _re60i = 0;
    double _re61r = 0, _re61i = 0;
    _re61r = x2r; _re61i = 0;
    double _prod62r = 0, _prod62i = 0;
    c_mul(_re60r, _re60i, _re61r, _re61i, &_prod62r, &_prod62i);
    double _im63r = 0, _im63i = 0;
    _im63r = x1i; _im63i = 0;
    double _im64r = 0, _im64i = 0;
    _im64r = x2i; _im64i = 0;
    double _prod65r = 0, _prod65i = 0;
    c_mul(_im63r, _im63i, _im64r, _im64i, &_prod65r, &_prod65i);
    double _c66r = 0, _c66i = 0;
    _c66r = 0.0; _c66i = 1.0;
    double _mul67r = 0, _mul67i = 0;
    c_mul(_prod65r, _prod65i, _c66r, _c66i, &_mul67r, &_mul67i);
    double _add68r = 0, _add68i = 0;
    _add68r = _prod62r + _mul67r; _add68i = _prod62i + _mul67i;
    { int _idx = 19; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add68r; cIm[_idx] = _add68i; } }
    double _abs69r = 0, _abs69i = 0;
    _abs69r = c_abs(x1r, x1i); _abs69i = 0;
    double _abs70r = 0, _abs70i = 0;
    _abs70r = c_abs(x2r, x2i); _abs70i = 0;
    double _sum71r = 0, _sum71i = 0;
    _sum71r = _abs69r + _abs70r; _sum71i = _abs69i + _abs70i;
    double _add72r = 0, _add72i = 0;
    _add72r = x1r + x2r; _add72i = x1i + x2i;
    double _ang73r = 0, _ang73i = 0;
    _ang73r = c_arg(_add72r, _add72i); _ang73i = 0;
    double _c74r = 0, _c74i = 0;
    _c74r = 0.0; _c74i = 1.0;
    double _mul75r = 0, _mul75i = 0;
    c_mul(_ang73r, _ang73i, _c74r, _c74i, &_mul75r, &_mul75i);
    double _add76r = 0, _add76i = 0;
    _add76r = _sum71r + _mul75r; _add76i = _sum71i + _mul75i;
    { int _idx = 24; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add76r; cIm[_idx] = _add76i; } }
    double _abs77r = 0, _abs77i = 0;
    _abs77r = c_abs(x1r, x1i); _abs77i = 0;
    double _sin78r = 0, _sin78i = 0;
    c_sin(_abs77r, _abs77i, &_sin78r, &_sin78i);
    double _abs79r = 0, _abs79i = 0;
    _abs79r = c_abs(x2r, x2i); _abs79i = 0;
    double _cos80r = 0, _cos80i = 0;
    c_cos(_abs79r, _abs79i, &_cos80r, &_cos80i);
    double _c81r = 0, _c81i = 0;
    _c81r = 0.0; _c81i = 1.0;
    double _mul82r = 0, _mul82i = 0;
    c_mul(_cos80r, _cos80i, _c81r, _c81i, &_mul82r, &_mul82i);
    double _add83r = 0, _add83i = 0;
    _add83r = _sin78r + _mul82r; _add83i = _sin78i + _mul82i;
    { int _idx = 27; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add83r; cIm[_idx] = _add83i; } }
    double _abs84r = 0, _abs84i = 0;
    _abs84r = c_abs(x1r, x1i); _abs84i = 0;
    double _c85r = 0, _c85i = 0;
    _c85r = 1.0; _c85i = 0;
    double _add86r = 0, _add86i = 0;
    _add86r = _abs84r + _c85r; _add86i = _abs84i + _c85i;
    double _log87r = 0, _log87i = 0;
    c_log(_add86r, _add86i, &_log87r, &_log87i);
    double _abs88r = 0, _abs88i = 0;
    _abs88r = c_abs(x2r, x2i); _abs88i = 0;
    double _c89r = 0, _c89i = 0;
    _c89r = 1.0; _c89i = 0;
    double _add90r = 0, _add90i = 0;
    _add90r = _abs88r + _c89r; _add90i = _abs88i + _c89i;
    double _log91r = 0, _log91i = 0;
    c_log(_add90r, _add90i, &_log91r, &_log91i);
    double _c92r = 0, _c92i = 0;
    _c92r = 0.0; _c92i = 1.0;
    double _mul93r = 0, _mul93i = 0;
    c_mul(_log91r, _log91i, _c92r, _c92i, &_mul93r, &_mul93i);
    double _add94r = 0, _add94i = 0;
    _add94r = _log87r + _mul93r; _add94i = _log87i + _mul93i;
    { int _idx = 31; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add94r; cIm[_idx] = _add94i; } }
    double _add95r = 0, _add95i = 0;
    _add95r = x1r + x2r; _add95i = x1i + x2i;
    double _conj96r = 0, _conj96i = 0;
    _conj96r = _add95r; _conj96i = -(_add95i);
    { int _idx = 34; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _conj96r; cIm[_idx] = _conj96i; } }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_274_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
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
        double _pow5r = 0, _pow5i = 0;
        c_powr(_log4r, _log4i, j, &_pow5r, &_pow5i);
        double _ang6r = 0, _ang6i = 0;
        _ang6r = c_arg(x1r, x1i); _ang6i = 0;
        double _mul7r = 0, _mul7i = 0;
        c_mul(j, 0, _ang6r, _ang6i, &_mul7r, &_mul7i);
        double _sin8r = 0, _sin8i = 0;
        c_sin(_mul7r, _mul7i, &_sin8r, &_sin8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(_pow5r, _pow5i, _sin8r, _sin8i, &_mul9r, &_mul9i);
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(x2r, x2i); _abs10i = 0;
        double _c11r = 0, _c11i = 0;
        _c11r = 4.0; _c11i = 0;
        double _mod12r = 0, _mod12i = 0;
        _mod12r = fmod(j, _c11r); _mod12i = 0;
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = _mod12r + _c13r; _add14i = _mod12i + _c13i;
        double _pow15r = 0, _pow15i = 0;
        c_powc(_abs10r, _abs10i, _add14r, _add14i, &_pow15r, &_pow15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _mul9r + _pow15r; _add16i = _mul9i + _pow15i;
        double mag_r = _add16r, mag_i = _add16i;
        double _ang17r = 0, _ang17i = 0;
        _ang17r = c_arg(x1r, x1i); _ang17i = 0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(_ang17r, _ang17i, j, 0, &_mul18r, &_mul18i);
        double _ang19r = 0, _ang19i = 0;
        _ang19r = c_arg(x2r, x2i); _ang19i = 0;
        double _c20r = 0, _c20i = 0;
        _c20r = 5.0; _c20i = 0;
        double _mod21r = 0, _mod21i = 0;
        _mod21r = fmod(j, _c20r); _mod21i = 0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(_ang19r, _ang19i, _mod21r, _mod21i, &_mul22r, &_mul22i);
        double _add23r = 0, _add23i = 0;
        _add23r = _mul18r + _mul22r; _add23i = _mul18i + _mul22i;
        double ang_r = _add23r, ang_i = _add23i;
        double _cos24r = 0, _cos24i = 0;
        c_cos(ang_r, ang_i, &_cos24r, &_cos24i);
        double _c25r = 0, _c25i = 0;
        _c25r = 0.0; _c25i = 1.0;
        double _sin26r = 0, _sin26i = 0;
        c_sin(ang_r, ang_i, &_sin26r, &_sin26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_c25r, _c25i, _sin26r, _sin26i, &_mul27r, &_mul27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _cos24r + _mul27r; _add28i = _cos24i + _mul27i;
        double _mul29r = 0, _mul29i = 0;
        c_mul(mag_r, mag_i, _add28r, _add28i, &_mul29r, &_mul29i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul29r; cIm[_idx] = _mul29i; } }
    }
    for (int k = 1; k < 36; k += 7) {
        double _c30r = 0, _c30i = 0;
        _c30r = 0.0; _c30i = 100.0;
        double _pow31r = 0, _pow31i = 0;
        c_powr(x1r, x1i, k, &_pow31r, &_pow31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_c30r, _c30i, _pow31r, _pow31i, &_mul32r, &_mul32i);
        double _c33r = 0, _c33i = 0;
        _c33r = 50.0; _c33i = 0;
        double _c34r = 0, _c34i = 0;
        _c34r = 3.0; _c34i = 0;
        double _mod35r = 0, _mod35i = 0;
        _mod35r = fmod(k, _c34r); _mod35i = 0;
        double _pow36r = 0, _pow36i = 0;
        c_powc(x2r, x2i, _mod35r, _mod35i, &_pow36r, &_pow36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(_c33r, _c33i, _pow36r, _pow36i, &_mul37r, &_mul37i);
        double _sub38r = 0, _sub38i = 0;
        _sub38r = _mul32r - _mul37r; _sub38i = _mul32i - _mul37i;
        cRe[(k - 1)] += _sub38r; cIm[(k - 1)] += _sub38i;
    }
    for (int r = 2; r < 35; r++) {
        double _cf39r = 0, _cf39i = 0;
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 35) { _cf39r = cRe[_idx]; _cf39i = cIm[_idx]; } }
        double _c40r = 0, _c40i = 0;
        _c40r = 1.0; _c40i = 0;
        double _c41r = 0, _c41i = 0;
        _c41r = 0.1; _c41i = 0;
        double _mul42r = 0, _mul42i = 0;
        c_mul(_c41r, _c41i, r, 0, &_mul42r, &_mul42i);
        double _add43r = 0, _add43i = 0;
        _add43r = _c40r + _mul42r; _add43i = _c40i + _mul42i;
        double _mul44r = 0, _mul44i = 0;
        c_mul(_cf39r, _cf39i, _add43r, _add43i, &_mul44r, &_mul44i);
        double _conj45r = 0, _conj45i = 0;
        _conj45r = x1r; _conj45i = -(x1i);
        double _ang46r = 0, _ang46i = 0;
        _ang46r = c_arg(x2r, x2i); _ang46i = 0;
        double _mul47r = 0, _mul47i = 0;
        c_mul(r, 0, _ang46r, _ang46i, &_mul47r, &_mul47i);
        double _sin48r = 0, _sin48i = 0;
        c_sin(_mul47r, _mul47i, &_sin48r, &_sin48i);
        double _mul49r = 0, _mul49i = 0;
        c_mul(_conj45r, _conj45i, _sin48r, _sin48i, &_mul49r, &_mul49i);
        double _add50r = 0, _add50i = 0;
        _add50r = _mul44r + _mul49r; _add50i = _mul44i + _mul49i;
        { int _idx = (r - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add50r; cIm[_idx] = _add50i; } }
    }
    double _c51r = 0, _c51i = 0;
    _c51r = 1.0; _c51i = 0;
    double _re52r = 0, _re52i = 0;
    _re52r = x1r; _re52i = 0;
    double _add53r = 0, _add53i = 0;
    _add53r = _c51r + _re52r; _add53i = _c51i + _re52i;
    double _re54r = 0, _re54i = 0;
    _re54r = x2r; _re54i = 0;
    double _sub55r = 0, _sub55i = 0;
    _sub55r = _add53r - _re54r; _sub55i = _add53i - _re54i;
    { int _idx = 0; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub55r; cIm[_idx] = _sub55i; } }
    double _c56r = 0, _c56i = 0;
    _c56r = 2.0; _c56i = 0;
    double _im57r = 0, _im57i = 0;
    _im57r = x1i; _im57i = 0;
    double _sub58r = 0, _sub58i = 0;
    _sub58r = _c56r - _im57r; _sub58i = _c56i - _im57i;
    double _im59r = 0, _im59i = 0;
    _im59r = x2i; _im59i = 0;
    double _add60r = 0, _add60i = 0;
    _add60r = _sub58r + _im59r; _add60i = _sub58i + _im59i;
    { int _idx = 34; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add60r; cIm[_idx] = _add60i; } }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_275_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 36; j++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 4.0; _c1i = 0;
        double _mul2r = 0, _mul2i = 0;
        c_mul(j, 0, _c1r, _c1i, &_mul2r, &_mul2i);
        double _c3r = 0, _c3i = 0;
        _c3r = 8.0; _c3i = 0;
        double _mod4r = 0, _mod4i = 0;
        _mod4r = fmod(_mul2r, _c3r); _mod4i = 0;
        double _c5r = 0, _c5i = 0;
        _c5r = 1.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = _mod4r + _c5r; _add6i = _mod4i + _c5i;
        double k_r = _add6r, k_i = _add6i;
        double _c7r = 0, _c7i = 0;
        _c7r = 5.0; _c7i = 0;
        double _fdiv8r = 0, _fdiv8i = 0;
        c_div(j, 0, _c7r, _c7i, &_fdiv8r, &_fdiv8i);
        _fdiv8r = floor(_fdiv8r); _fdiv8i = 0;
        double _c9r = 0, _c9i = 0;
        _c9r = 2.0; _c9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = _fdiv8r + _c9r; _add10i = _fdiv8i + _c9i;
        double r_r = _add10r, r_i = _add10i;
        double _ang11r = 0, _ang11i = 0;
        _ang11r = c_arg(x1r, x1i); _ang11i = 0;
        double _mul12r = 0, _mul12i = 0;
        c_mul(_ang11r, _ang11i, j, 0, &_mul12r, &_mul12i);
        double _ang13r = 0, _ang13i = 0;
        _ang13r = c_arg(x2r, x2i); _ang13i = 0;
        double _mul14r = 0, _mul14i = 0;
        c_mul(_ang13r, _ang13i, k_r, k_i, &_mul14r, &_mul14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _mul12r + _mul14r; _add15i = _mul12i + _mul14i;
        double _sin16r = 0, _sin16i = 0;
        c_sin(j, 0, &_sin16r, &_sin16i);
        double _cos17r = 0, _cos17i = 0;
        c_cos(k_r, k_i, &_cos17r, &_cos17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_sin16r, _sin16i, _cos17r, _cos17i, &_mul18r, &_mul18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _add15r + _mul18r; _add19i = _add15i + _mul18i;
        double angle_r = _add19r, angle_i = _add19i;
        double _abs20r = 0, _abs20i = 0;
        _abs20r = c_abs(x1r, x1i); _abs20i = 0;
        double _pow21r = 0, _pow21i = 0;
        c_powr(_abs20r, _abs20i, j, &_pow21r, &_pow21i);
        double _abs22r = 0, _abs22i = 0;
        _abs22r = c_abs(x2r, x2i); _abs22i = 0;
        double _pow23r = 0, _pow23i = 0;
        c_powc(_abs22r, _abs22i, k_r, k_i, &_pow23r, &_pow23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _pow21r + _pow23r; _add24i = _pow21i + _pow23i;
        double _mul25r = 0, _mul25i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul25r, &_mul25i);
        double _abs26r = 0, _abs26i = 0;
        _abs26r = c_abs(_mul25r, _mul25i); _abs26i = 0;
        double _c27r = 0, _c27i = 0;
        _c27r = 1.0; _c27i = 0;
        double _add28r = 0, _add28i = 0;
        _add28r = _abs26r + _c27r; _add28i = _abs26i + _c27i;
        double _log29r = 0, _log29i = 0;
        c_log(_add28r, _add28i, &_log29r, &_log29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_log29r, _log29i, r_r, r_i, &_mul30r, &_mul30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _add24r + _mul30r; _add31i = _add24i + _mul30i;
        double mag_r = _add31r, mag_i = _add31i;
        double _cos32r = 0, _cos32i = 0;
        c_cos(angle_r, angle_i, &_cos32r, &_cos32i);
        double _c33r = 0, _c33i = 0;
        _c33r = 0.0; _c33i = 1.0;
        double _sin34r = 0, _sin34i = 0;
        c_sin(angle_r, angle_i, &_sin34r, &_sin34i);
        double _mul35r = 0, _mul35i = 0;
        c_mul(_c33r, _c33i, _sin34r, _sin34i, &_mul35r, &_mul35i);
        double _add36r = 0, _add36i = 0;
        _add36r = _cos32r + _mul35r; _add36i = _cos32i + _mul35i;
        double _mul37r = 0, _mul37i = 0;
        c_mul(mag_r, mag_i, _add36r, _add36i, &_mul37r, &_mul37i);
        double _conj38r = 0, _conj38i = 0;
        _conj38r = x1r; _conj38i = -(x1i);
        double _pow39r = 0, _pow39i = 0;
        c_powc(_conj38r, _conj38i, r_r, r_i, &_pow39r, &_pow39i);
        double _conj40r = 0, _conj40i = 0;
        _conj40r = x2r; _conj40i = -(x2i);
        double _pow41r = 0, _pow41i = 0;
        c_powc(_conj40r, _conj40i, k_r, k_i, &_pow41r, &_pow41i);
        double _mul42r = 0, _mul42i = 0;
        c_mul(_pow39r, _pow39i, _pow41r, _pow41i, &_mul42r, &_mul42i);
        double _add43r = 0, _add43i = 0;
        _add43r = _mul37r + _mul42r; _add43i = _mul37i + _mul42i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add43r; cIm[_idx] = _add43i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_276_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 1; j < 36; j++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 5.0; _c1i = 0;
        double _mod2r = 0, _mod2i = 0;
        _mod2r = fmod(j, _c1r); _mod2i = 0;
        double _c3r = 0, _c3i = 0;
        _c3r = 1.0; _c3i = 0;
        if (_mod2r == _c3r) {
            double _re4r = 0, _re4i = 0;
            _re4r = x1r; _re4i = 0;
            double _pow5r = 0, _pow5i = 0;
            c_powr(_re4r, _re4i, j, &_pow5r, &_pow5i);
            double _im6r = 0, _im6i = 0;
            _im6r = x2i; _im6i = 0;
            double _c7r = 0, _c7i = 0;
            _c7r = 3.0; _c7i = 0;
            double _mod8r = 0, _mod8i = 0;
            _mod8r = fmod(j, _c7r); _mod8i = 0;
            double _c9r = 0, _c9i = 0;
            _c9r = 1.0; _c9i = 0;
            double _add10r = 0, _add10i = 0;
            _add10r = _mod8r + _c9r; _add10i = _mod8i + _c9i;
            double _pow11r = 0, _pow11i = 0;
            c_powc(_im6r, _im6i, _add10r, _add10i, &_pow11r, &_pow11i);
            double _conj12r = 0, _conj12i = 0;
            _conj12r = x1r; _conj12i = -(x1i);
            double _mul13r = 0, _mul13i = 0;
            c_mul(_pow11r, _pow11i, _conj12r, _conj12i, &_mul13r, &_mul13i);
            double _add14r = 0, _add14i = 0;
            _add14r = _pow5r + _mul13r; _add14i = _pow5i + _mul13i;
            { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add14r; cIm[_idx] = _add14i; } }
        } else {
            double _c15r = 0, _c15i = 0;
            _c15r = 5.0; _c15i = 0;
            double _mod16r = 0, _mod16i = 0;
            _mod16r = fmod(j, _c15r); _mod16i = 0;
            double _c17r = 0, _c17i = 0;
            _c17r = 2.0; _c17i = 0;
            if (_mod16r == _c17r) {
                double _add18r = 0, _add18i = 0;
                _add18r = x1r + x2r; _add18i = x1i + x2i;
                double _abs19r = 0, _abs19i = 0;
                _abs19r = c_abs(_add18r, _add18i); _abs19i = 0;
                double _pow20r = 0, _pow20i = 0;
                c_powr(_abs19r, _abs19i, j, &_pow20r, &_pow20i);
                double _ang21r = 0, _ang21i = 0;
                _ang21r = c_arg(x1r, x1i); _ang21i = 0;
                double _mul22r = 0, _mul22i = 0;
                c_mul(_ang21r, _ang21i, j, 0, &_mul22r, &_mul22i);
                double _sin23r = 0, _sin23i = 0;
                c_sin(_mul22r, _mul22i, &_sin23r, &_sin23i);
                double _mul24r = 0, _mul24i = 0;
                c_mul(_pow20r, _pow20i, _sin23r, _sin23i, &_mul24r, &_mul24i);
                double _c25r = 0, _c25i = 0;
                _c25r = 0.0; _c25i = 1.0;
                double _ang26r = 0, _ang26i = 0;
                _ang26r = c_arg(x2r, x2i); _ang26i = 0;
                double _mul27r = 0, _mul27i = 0;
                c_mul(_ang26r, _ang26i, j, 0, &_mul27r, &_mul27i);
                double _cos28r = 0, _cos28i = 0;
                c_cos(_mul27r, _mul27i, &_cos28r, &_cos28i);
                double _mul29r = 0, _mul29i = 0;
                c_mul(_c25r, _c25i, _cos28r, _cos28i, &_mul29r, &_mul29i);
                double _add30r = 0, _add30i = 0;
                _add30r = _mul24r + _mul29r; _add30i = _mul24i + _mul29i;
                { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add30r; cIm[_idx] = _add30i; } }
            } else {
                double _c31r = 0, _c31i = 0;
                _c31r = 5.0; _c31i = 0;
                double _mod32r = 0, _mod32i = 0;
                _mod32r = fmod(j, _c31r); _mod32i = 0;
                double _c33r = 0, _c33i = 0;
                _c33r = 3.0; _c33i = 0;
                if (_mod32r == _c33r) {
                    double _abs34r = 0, _abs34i = 0;
                    _abs34r = c_abs(x1r, x1i); _abs34i = 0;
                    double _c35r = 0, _c35i = 0;
                    _c35r = 1.0; _c35i = 0;
                    double _add36r = 0, _add36i = 0;
                    _add36r = _abs34r + _c35r; _add36i = _abs34i + _c35i;
                    double _log37r = 0, _log37i = 0;
                    c_log(_add36r, _add36i, &_log37r, &_log37i);
                    double _re38r = 0, _re38i = 0;
                    _re38r = x2r; _re38i = 0;
                    double _pow39r = 0, _pow39i = 0;
                    c_powr(_re38r, _re38i, j, &_pow39r, &_pow39i);
                    double _mul40r = 0, _mul40i = 0;
                    c_mul(_log37r, _log37i, _pow39r, _pow39i, &_mul40r, &_mul40i);
                    double _c41r = 0, _c41i = 0;
                    _c41r = 0.0; _c41i = 1.0;
                    double _abs42r = 0, _abs42i = 0;
                    _abs42r = c_abs(x2r, x2i); _abs42i = 0;
                    double _c43r = 0, _c43i = 0;
                    _c43r = 1.0; _c43i = 0;
                    double _add44r = 0, _add44i = 0;
                    _add44r = _abs42r + _c43r; _add44i = _abs42i + _c43i;
                    double _log45r = 0, _log45i = 0;
                    c_log(_add44r, _add44i, &_log45r, &_log45i);
                    double _mul46r = 0, _mul46i = 0;
                    c_mul(_c41r, _c41i, _log45r, _log45i, &_mul46r, &_mul46i);
                    double _im47r = 0, _im47i = 0;
                    _im47r = x1i; _im47i = 0;
                    double _pow48r = 0, _pow48i = 0;
                    c_powr(_im47r, _im47i, j, &_pow48r, &_pow48i);
                    double _mul49r = 0, _mul49i = 0;
                    c_mul(_mul46r, _mul46i, _pow48r, _pow48i, &_mul49r, &_mul49i);
                    double _sub50r = 0, _sub50i = 0;
                    _sub50r = _mul40r - _mul49r; _sub50i = _mul40i - _mul49i;
                    { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub50r; cIm[_idx] = _sub50i; } }
                } else {
                    double _c51r = 0, _c51i = 0;
                    _c51r = 5.0; _c51i = 0;
                    double _mod52r = 0, _mod52i = 0;
                    _mod52r = fmod(j, _c51r); _mod52i = 0;
                    double _c53r = 0, _c53i = 0;
                    _c53r = 4.0; _c53i = 0;
                    if (_mod52r == _c53r) {
                        double _c54r = 0, _c54i = 0;
                        _c54r = 2.0; _c54i = 0;
                        double _pow55r = 0, _pow55i = 0;
                        c_mul(x1r, x1i, x1r, x1i, &_pow55r, &_pow55i);
                        double _c56r = 0, _c56i = 0;
                        _c56r = 3.0; _c56i = 0;
                        double _pow57r = 0, _pow57i = 0;
                        c_mul(x2r, x2i, x2r, x2i, &_pow57r, &_pow57i);
                        c_mul(_pow57r, _pow57i, x2r, x2i, &_pow57r, &_pow57i);
                        double _add58r = 0, _add58i = 0;
                        _add58r = _pow55r + _pow57r; _add58i = _pow55i + _pow57i;
                        double _sin59r = 0, _sin59i = 0;
                        c_sin(j, 0, &_sin59r, &_sin59i);
                        double _mul60r = 0, _mul60i = 0;
                        c_mul(_add58r, _add58i, _sin59r, _sin59i, &_mul60r, &_mul60i);
                        double _c61r = 0, _c61i = 0;
                        _c61r = 0.0; _c61i = 1.0;
                        double _mul62r = 0, _mul62i = 0;
                        c_mul(x1r, x1i, x2r, x2i, &_mul62r, &_mul62i);
                        double _c63r = 0, _c63i = 0;
                        _c63r = 2.0; _c63i = 0;
                        double _pow64r = 0, _pow64i = 0;
                        c_mul(_mul62r, _mul62i, _mul62r, _mul62i, &_pow64r, &_pow64i);
                        double _mul65r = 0, _mul65i = 0;
                        c_mul(_c61r, _c61i, _pow64r, _pow64i, &_mul65r, &_mul65i);
                        double _cos66r = 0, _cos66i = 0;
                        c_cos(j, 0, &_cos66r, &_cos66i);
                        double _mul67r = 0, _mul67i = 0;
                        c_mul(_mul65r, _mul65i, _cos66r, _cos66i, &_mul67r, &_mul67i);
                        double _add68r = 0, _add68i = 0;
                        _add68r = _mul60r + _mul67r; _add68i = _mul60i + _mul67i;
                        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add68r; cIm[_idx] = _add68i; } }
                    } else {
                        double _re69r = 0, _re69i = 0;
                        _re69r = x1r; _re69i = 0;
                        double _im70r = 0, _im70i = 0;
                        _im70r = x2i; _im70i = 0;
                        double _prod71r = 0, _prod71i = 0;
                        c_mul(_re69r, _re69i, _im70r, _im70i, &_prod71r, &_prod71i);
                        double _prod72r = 0, _prod72i = 0;
                        c_mul(_prod71r, _prod71i, j, 0, &_prod72r, &_prod72i);
                        double _c73r = 0, _c73i = 0;
                        _c73r = 0.0; _c73i = 1.0;
                        double _abs74r = 0, _abs74i = 0;
                        _abs74r = c_abs(x1r, x1i); _abs74i = 0;
                        double _abs75r = 0, _abs75i = 0;
                        _abs75r = c_abs(x2r, x2i); _abs75i = 0;
                        double _sum76r = 0, _sum76i = 0;
                        _sum76r = _abs74r + _abs75r; _sum76i = _abs74i + _abs75i;
                        double _sum77r = 0, _sum77i = 0;
                        _sum77r = _sum76r + j; _sum77i = _sum76i + 0;
                        double _mul78r = 0, _mul78i = 0;
                        c_mul(_c73r, _c73i, _sum77r, _sum77i, &_mul78r, &_mul78i);
                        double _add79r = 0, _add79i = 0;
                        _add79r = _prod72r + _mul78r; _add79i = _prod72i + _mul78i;
                        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add79r; cIm[_idx] = _add79i; } }
                    }
                }
            }
        }
    }
    double _c80r = 0, _c80i = 0;
    _c80r = 100.0; _c80i = 0;
    double _c81r = 0, _c81i = 0;
    _c81r = 4.0; _c81i = 0;
    double _pow82r = 0, _pow82i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow82r, &_pow82i);
    c_mul(_pow82r, _pow82i, _pow82r, _pow82i, &_pow82r, &_pow82i);
    double _mul83r = 0, _mul83i = 0;
    c_mul(_c80r, _c80i, _pow82r, _pow82i, &_mul83r, &_mul83i);
    double _c84r = 0, _c84i = 0;
    _c84r = 0.0; _c84i = 50.0;
    double _c85r = 0, _c85i = 0;
    _c85r = 2.0; _c85i = 0;
    double _pow86r = 0, _pow86i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow86r, &_pow86i);
    double _mul87r = 0, _mul87i = 0;
    c_mul(_c84r, _c84i, _pow86r, _pow86i, &_mul87r, &_mul87i);
    double _sub88r = 0, _sub88i = 0;
    _sub88r = _mul83r - _mul87r; _sub88i = _mul83i - _mul87i;
    double _c89r = 0, _c89i = 0;
    _c89r = 25.0; _c89i = 0;
    double _add90r = 0, _add90i = 0;
    _add90r = _sub88r + _c89r; _add90i = _sub88i + _c89i;
    { int _idx = 4; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add90r; cIm[_idx] = _add90i; } }
    double _c91r = 0, _c91i = 0;
    _c91r = 0.0; _c91i = 200.0;
    double _sin92r = 0, _sin92i = 0;
    c_sin(x1r, x1i, &_sin92r, &_sin92i);
    double _mul93r = 0, _mul93i = 0;
    c_mul(_c91r, _c91i, _sin92r, _sin92i, &_mul93r, &_mul93i);
    double _c94r = 0, _c94i = 0;
    _c94r = 150.0; _c94i = 0;
    double _cos95r = 0, _cos95i = 0;
    c_cos(x2r, x2i, &_cos95r, &_cos95i);
    double _mul96r = 0, _mul96i = 0;
    c_mul(_c94r, _c94i, _cos95r, _cos95i, &_mul96r, &_mul96i);
    double _add97r = 0, _add97i = 0;
    _add97r = _mul93r + _mul96r; _add97i = _mul93i + _mul96i;
    { int _idx = 9; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add97r; cIm[_idx] = _add97i; } }
    double _c98r = 0, _c98i = 0;
    _c98r = 300.0; _c98i = 0;
    double _abs99r = 0, _abs99i = 0;
    _abs99r = c_abs(x1r, x1i); _abs99i = 0;
    double _c100r = 0, _c100i = 0;
    _c100r = 1.0; _c100i = 0;
    double _add101r = 0, _add101i = 0;
    _add101r = _abs99r + _c100r; _add101i = _abs99i + _c100i;
    double _log102r = 0, _log102i = 0;
    c_log(_add101r, _add101i, &_log102r, &_log102i);
    double _mul103r = 0, _mul103i = 0;
    c_mul(_c98r, _c98i, _log102r, _log102i, &_mul103r, &_mul103i);
    double _c104r = 0, _c104i = 0;
    _c104r = 0.0; _c104i = 100.0;
    double _abs105r = 0, _abs105i = 0;
    _abs105r = c_abs(x2r, x2i); _abs105i = 0;
    double _c106r = 0, _c106i = 0;
    _c106r = 1.0; _c106i = 0;
    double _add107r = 0, _add107i = 0;
    _add107r = _abs105r + _c106r; _add107i = _abs105i + _c106i;
    double _log108r = 0, _log108i = 0;
    c_log(_add107r, _add107i, &_log108r, &_log108i);
    double _mul109r = 0, _mul109i = 0;
    c_mul(_c104r, _c104i, _log108r, _log108i, &_mul109r, &_mul109i);
    double _add110r = 0, _add110i = 0;
    _add110r = _mul103r + _mul109r; _add110i = _mul103i + _mul109i;
    { int _idx = 14; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add110r; cIm[_idx] = _add110i; } }
    double _conj111r = 0, _conj111i = 0;
    _conj111r = x1r; _conj111i = -(x1i);
    double _c112r = 0, _c112i = 0;
    _c112r = 3.0; _c112i = 0;
    double _pow113r = 0, _pow113i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow113r, &_pow113i);
    c_mul(_pow113r, _pow113i, x2r, x2i, &_pow113r, &_pow113i);
    double _mul114r = 0, _mul114i = 0;
    c_mul(_conj111r, _conj111i, _pow113r, _pow113i, &_mul114r, &_mul114i);
    double _c115r = 0, _c115i = 0;
    _c115r = 2.0; _c115i = 0;
    double _pow116r = 0, _pow116i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow116r, &_pow116i);
    double _conj117r = 0, _conj117i = 0;
    _conj117r = x2r; _conj117i = -(x2i);
    double _mul118r = 0, _mul118i = 0;
    c_mul(_pow116r, _pow116i, _conj117r, _conj117i, &_mul118r, &_mul118i);
    double _sub119r = 0, _sub119i = 0;
    _sub119r = _mul114r - _mul118r; _sub119i = _mul114i - _mul118i;
    { int _idx = 19; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub119r; cIm[_idx] = _sub119i; } }
    double _abs120r = 0, _abs120i = 0;
    _abs120r = c_abs(x1r, x1i); _abs120i = 0;
    double _c121r = 0, _c121i = 0;
    _c121r = 3.0; _c121i = 0;
    double _pow122r = 0, _pow122i = 0;
    c_mul(_abs120r, _abs120i, _abs120r, _abs120i, &_pow122r, &_pow122i);
    c_mul(_pow122r, _pow122i, _abs120r, _abs120i, &_pow122r, &_pow122i);
    double _abs123r = 0, _abs123i = 0;
    _abs123r = c_abs(x2r, x2i); _abs123i = 0;
    double _c124r = 0, _c124i = 0;
    _c124r = 2.0; _c124i = 0;
    double _pow125r = 0, _pow125i = 0;
    c_mul(_abs123r, _abs123i, _abs123r, _abs123i, &_pow125r, &_pow125i);
    double _c126r = 0, _c126i = 0;
    _c126r = 0.0; _c126i = 1.0;
    double _mul127r = 0, _mul127i = 0;
    c_mul(_pow125r, _pow125i, _c126r, _c126i, &_mul127r, &_mul127i);
    double _add128r = 0, _add128i = 0;
    _add128r = _pow122r + _mul127r; _add128i = _pow122i + _mul127i;
    { int _idx = 24; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add128r; cIm[_idx] = _add128i; } }
    double _mul129r = 0, _mul129i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul129r, &_mul129i);
    double _sin130r = 0, _sin130i = 0;
    c_sin(_mul129r, _mul129i, &_sin130r, &_sin130i);
    double _add131r = 0, _add131i = 0;
    _add131r = x1r + x2r; _add131i = x1i + x2i;
    double _cos132r = 0, _cos132i = 0;
    c_cos(_add131r, _add131i, &_cos132r, &_cos132i);
    double _c133r = 0, _c133i = 0;
    _c133r = 0.0; _c133i = 1.0;
    double _mul134r = 0, _mul134i = 0;
    c_mul(_cos132r, _cos132i, _c133r, _c133i, &_mul134r, &_mul134i);
    double _add135r = 0, _add135i = 0;
    _add135r = _sin130r + _mul134r; _add135i = _sin130i + _mul134i;
    { int _idx = 29; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add135r; cIm[_idx] = _add135i; } }
    double _mul136r = 0, _mul136i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul136r, &_mul136i);
    double _abs137r = 0, _abs137i = 0;
    _abs137r = c_abs(_mul136r, _mul136i); _abs137i = 0;
    double _c138r = 0, _c138i = 0;
    _c138r = 1.0; _c138i = 0;
    double _add139r = 0, _add139i = 0;
    _add139r = _abs137r + _c138r; _add139i = _abs137i + _c138i;
    double _log140r = 0, _log140i = 0;
    c_log(_add139r, _add139i, &_log140r, &_log140i);
    double _c141r = 0, _c141i = 0;
    _c141r = 0.0; _c141i = 1.0;
    double _add142r = 0, _add142i = 0;
    _add142r = x1r + x2r; _add142i = x1i + x2i;
    double _ang143r = 0, _ang143i = 0;
    _ang143r = c_arg(_add142r, _add142i); _ang143i = 0;
    double _mul144r = 0, _mul144i = 0;
    c_mul(_c141r, _c141i, _ang143r, _ang143i, &_mul144r, &_mul144i);
    double _add145r = 0, _add145i = 0;
    _add145r = _log140r + _mul144r; _add145i = _log140i + _mul144i;
    { int _idx = 34; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add145r; cIm[_idx] = _add145i; } }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_277_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int j = 0; j < 35; j++) {
        double k = 0;
        double _c1r = 0, _c1i = 0;
        _c1r = 5.0; _c1i = 0;
        if (j < _c1r) {
            double _c2r = 0, _c2i = 0;
            _c2r = 1.0; _c2i = 0;
            double _add3r = 0, _add3i = 0;
            _add3r = j + _c2r; _add3i = 0 + _c2i;
            k = _add3r;
            double _re4r = 0, _re4i = 0;
            _re4r = x1r; _re4i = 0;
            double _pow5r = 0, _pow5i = 0;
            c_powr(_re4r, _re4i, k, &_pow5r, &_pow5i);
            double _im6r = 0, _im6i = 0;
            _im6r = x2i; _im6i = 0;
            double _pow7r = 0, _pow7i = 0;
            c_powr(_im6r, _im6i, k, &_pow7r, &_pow7i);
            double _add8r = 0, _add8i = 0;
            _add8r = _pow5r + _pow7r; _add8i = _pow5i + _pow7i;
            double _c9r = 0, _c9i = 0;
            _c9r = 0.0; _c9i = 1.0;
            double _add10r = 0, _add10i = 0;
            _add10r = x1r + x2r; _add10i = x1i + x2i;
            double _ang11r = 0, _ang11i = 0;
            _ang11r = c_arg(_add10r, _add10i); _ang11i = 0;
            double _mul12r = 0, _mul12i = 0;
            c_mul(_c9r, _c9i, _ang11r, _ang11i, &_mul12r, &_mul12i);
            double _exp13r = 0, _exp13i = 0;
            c_exp2(_mul12r, _mul12i, &_exp13r, &_exp13i);
            double _mul14r = 0, _mul14i = 0;
            c_mul(_add8r, _add8i, _exp13r, _exp13i, &_mul14r, &_mul14i);
            { int _idx = j; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul14r; cIm[_idx] = _mul14i; } }
        } else {
            double _c15r = 0, _c15i = 0;
            _c15r = 10.0; _c15i = 0;
            if (j < _c15r) {
                double _c16r = 0, _c16i = 0;
                _c16r = 4.0; _c16i = 0;
                double _sub17r = 0, _sub17i = 0;
                _sub17r = j - _c16r; _sub17i = 0 - _c16i;
                k = _sub17r;
                double _abs18r = 0, _abs18i = 0;
                _abs18r = c_abs(x1r, x1i); _abs18i = 0;
                double _pow19r = 0, _pow19i = 0;
                c_powr(_abs18r, _abs18i, k, &_pow19r, &_pow19i);
                double _abs20r = 0, _abs20i = 0;
                _abs20r = c_abs(x2r, x2i); _abs20i = 0;
                double _c21r = 0, _c21i = 0;
                _c21r = 5.0; _c21i = 0;
                double _pow22r = 0, _pow22i = 0;
                c_powr(_abs20r, _abs20i, 5.0, &_pow22r, &_pow22i);
                double _mul23r = 0, _mul23i = 0;
                c_mul(_pow19r, _pow19i, _pow22r, _pow22i, &_mul23r, &_mul23i);
                double _c24r = 0, _c24i = 0;
                _c24r = 1.0; _c24i = 0;
                double _add25r = 0, _add25i = 0;
                _add25r = k + _c24r; _add25i = 0 + _c24i;
                double _div26r = 0, _div26i = 0;
                c_div(_mul23r, _mul23i, _add25r, _add25i, &_div26r, &_div26i);
                double _c27r = 0, _c27i = 0;
                _c27r = 0.0; _c27i = 1.0;
                double _ang28r = 0, _ang28i = 0;
                _ang28r = c_arg(x1r, x1i); _ang28i = 0;
                double _mul29r = 0, _mul29i = 0;
                c_mul(k, 0, _ang28r, _ang28i, &_mul29r, &_mul29i);
                double _sin30r = 0, _sin30i = 0;
                c_sin(_mul29r, _mul29i, &_sin30r, &_sin30i);
                double _mul31r = 0, _mul31i = 0;
                c_mul(_c27r, _c27i, _sin30r, _sin30i, &_mul31r, &_mul31i);
                double _add32r = 0, _add32i = 0;
                _add32r = _div26r + _mul31r; _add32i = _div26i + _mul31i;
                { int _idx = j; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add32r; cIm[_idx] = _add32i; } }
            } else {
                double _c33r = 0, _c33i = 0;
                _c33r = 15.0; _c33i = 0;
                if (j < _c33r) {
                    double _c34r = 0, _c34i = 0;
                    _c34r = 9.0; _c34i = 0;
                    double _sub35r = 0, _sub35i = 0;
                    _sub35r = j - _c34r; _sub35i = 0 - _c34i;
                    k = _sub35r;
                    double _mul36r = 0, _mul36i = 0;
                    c_mul(x1r, x1i, x2r, x2i, &_mul36r, &_mul36i);
                    double _re37r = 0, _re37i = 0;
                    _re37r = _mul36r; _re37i = 0;
                    double _c38r = 0, _c38i = 0;
                    _c38r = 0.0; _c38i = 1.0;
                    double _pow39r = 0, _pow39i = 0;
                    c_powr(x1r, x1i, k, &_pow39r, &_pow39i);
                    double _pow40r = 0, _pow40i = 0;
                    c_powr(x2r, x2i, k, &_pow40r, &_pow40i);
                    double _add41r = 0, _add41i = 0;
                    _add41r = _pow39r + _pow40r; _add41i = _pow39i + _pow40i;
                    double _im42r = 0, _im42i = 0;
                    _im42r = _add41i; _im42i = 0;
                    double _mul43r = 0, _mul43i = 0;
                    c_mul(_c38r, _c38i, _im42r, _im42i, &_mul43r, &_mul43i);
                    double _add44r = 0, _add44i = 0;
                    _add44r = _re37r + _mul43r; _add44i = _re37i + _mul43i;
                    { int _idx = j; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add44r; cIm[_idx] = _add44i; } }
                } else {
                    double _c45r = 0, _c45i = 0;
                    _c45r = 20.0; _c45i = 0;
                    if (j < _c45r) {
                        double _c46r = 0, _c46i = 0;
                        _c46r = 14.0; _c46i = 0;
                        double _sub47r = 0, _sub47i = 0;
                        _sub47r = j - _c46r; _sub47i = 0 - _c46i;
                        k = _sub47r;
                        double _abs48r = 0, _abs48i = 0;
                        _abs48r = c_abs(x1r, x1i); _abs48i = 0;
                        double _c49r = 0, _c49i = 0;
                        _c49r = 1.0; _c49i = 0;
                        double _add50r = 0, _add50i = 0;
                        _add50r = _abs48r + _c49r; _add50i = _abs48i + _c49i;
                        double _log51r = 0, _log51i = 0;
                        c_log(_add50r, _add50i, &_log51r, &_log51i);
                        double _ang52r = 0, _ang52i = 0;
                        _ang52r = c_arg(x2r, x2i); _ang52i = 0;
                        double _mul53r = 0, _mul53i = 0;
                        c_mul(k, 0, _ang52r, _ang52i, &_mul53r, &_mul53i);
                        double _cos54r = 0, _cos54i = 0;
                        c_cos(_mul53r, _mul53i, &_cos54r, &_cos54i);
                        double _mul55r = 0, _mul55i = 0;
                        c_mul(_log51r, _log51i, _cos54r, _cos54i, &_mul55r, &_mul55i);
                        double _c56r = 0, _c56i = 0;
                        _c56r = 0.0; _c56i = 1.0;
                        double _abs57r = 0, _abs57i = 0;
                        _abs57r = c_abs(x2r, x2i); _abs57i = 0;
                        double _c58r = 0, _c58i = 0;
                        _c58r = 1.0; _c58i = 0;
                        double _add59r = 0, _add59i = 0;
                        _add59r = _abs57r + _c58r; _add59i = _abs57i + _c58i;
                        double _log60r = 0, _log60i = 0;
                        c_log(_add59r, _add59i, &_log60r, &_log60i);
                        double _mul61r = 0, _mul61i = 0;
                        c_mul(_c56r, _c56i, _log60r, _log60i, &_mul61r, &_mul61i);
                        double _ang62r = 0, _ang62i = 0;
                        _ang62r = c_arg(x1r, x1i); _ang62i = 0;
                        double _mul63r = 0, _mul63i = 0;
                        c_mul(k, 0, _ang62r, _ang62i, &_mul63r, &_mul63i);
                        double _sin64r = 0, _sin64i = 0;
                        c_sin(_mul63r, _mul63i, &_sin64r, &_sin64i);
                        double _mul65r = 0, _mul65i = 0;
                        c_mul(_mul61r, _mul61i, _sin64r, _sin64i, &_mul65r, &_mul65i);
                        double _add66r = 0, _add66i = 0;
                        _add66r = _mul55r + _mul65r; _add66i = _mul55i + _mul65i;
                        { int _idx = j; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add66r; cIm[_idx] = _add66i; } }
                    } else {
                        double _c67r = 0, _c67i = 0;
                        _c67r = 25.0; _c67i = 0;
                        if (j < _c67r) {
                            double _c68r = 0, _c68i = 0;
                            _c68r = 19.0; _c68i = 0;
                            double _sub69r = 0, _sub69i = 0;
                            _sub69r = j - _c68r; _sub69i = 0 - _c68i;
                            k = _sub69r;
                            double _conj70r = 0, _conj70i = 0;
                            _conj70r = x2r; _conj70i = -(x2i);
                            double _add71r = 0, _add71i = 0;
                            _add71r = x1r + _conj70r; _add71i = x1i + _conj70i;
                            double _pow72r = 0, _pow72i = 0;
                            c_powr(_add71r, _add71i, k, &_pow72r, &_pow72i);
                            double _conj73r = 0, _conj73i = 0;
                            _conj73r = x1r; _conj73i = -(x1i);
                            double _sub74r = 0, _sub74i = 0;
                            _sub74r = _conj73r - x2r; _sub74i = _conj73i - x2i;
                            double _pow75r = 0, _pow75i = 0;
                            c_powr(_sub74r, _sub74i, k, &_pow75r, &_pow75i);
                            double _add76r = 0, _add76i = 0;
                            _add76r = _pow72r + _pow75r; _add76i = _pow72i + _pow75i;
                            { int _idx = j; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add76r; cIm[_idx] = _add76i; } }
                        } else {
                            double _c77r = 0, _c77i = 0;
                            _c77r = 30.0; _c77i = 0;
                            if (j < _c77r) {
                                double _c78r = 0, _c78i = 0;
                                _c78r = 24.0; _c78i = 0;
                                double _sub79r = 0, _sub79i = 0;
                                _sub79r = j - _c78r; _sub79i = 0 - _c78i;
                                k = _sub79r;
                                double _re80r = 0, _re80i = 0;
                                _re80r = x1r; _re80i = 0;
                                double _pow81r = 0, _pow81i = 0;
                                c_powr(_re80r, _re80i, k, &_pow81r, &_pow81i);
                                double _im82r = 0, _im82i = 0;
                                _im82r = x2i; _im82i = 0;
                                double _pow83r = 0, _pow83i = 0;
                                c_powr(_im82r, _im82i, k, &_pow83r, &_pow83i);
                                double _mul84r = 0, _mul84i = 0;
                                c_mul(_pow81r, _pow81i, _pow83r, _pow83i, &_mul84r, &_mul84i);
                                double _c85r = 0, _c85i = 0;
                                _c85r = 0.0; _c85i = 1.0;
                                double _add86r = 0, _add86i = 0;
                                _add86r = x1r + x2r; _add86i = x1i + x2i;
                                double _abs87r = 0, _abs87i = 0;
                                _abs87r = c_abs(_add86r, _add86i); _abs87i = 0;
                                double _pow88r = 0, _pow88i = 0;
                                c_powr(_abs87r, _abs87i, k, &_pow88r, &_pow88i);
                                double _mul89r = 0, _mul89i = 0;
                                c_mul(_c85r, _c85i, _pow88r, _pow88i, &_mul89r, &_mul89i);
                                double _add90r = 0, _add90i = 0;
                                _add90r = _mul84r + _mul89r; _add90i = _mul84i + _mul89i;
                                { int _idx = j; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add90r; cIm[_idx] = _add90i; } }
                            } else {
                                double _c91r = 0, _c91i = 0;
                                _c91r = 29.0; _c91i = 0;
                                double _sub92r = 0, _sub92i = 0;
                                _sub92r = j - _c91r; _sub92i = 0 - _c91i;
                                k = _sub92r;
                                double _re93r = 0, _re93i = 0;
                                _re93r = x1r; _re93i = 0;
                                double _im94r = 0, _im94i = 0;
                                _im94r = x2i; _im94i = 0;
                                double _mul95r = 0, _mul95i = 0;
                                c_mul(_re93r, _re93i, _im94r, _im94i, &_mul95r, &_mul95i);
                                double _pow96r = 0, _pow96i = 0;
                                c_powr(_mul95r, _mul95i, k, &_pow96r, &_pow96i);
                                double _mul97r = 0, _mul97i = 0;
                                c_mul(x1r, x1i, x2r, x2i, &_mul97r, &_mul97i);
                                double _conj98r = 0, _conj98i = 0;
                                _conj98r = _mul97r; _conj98i = -(_mul97i);
                                double _pow99r = 0, _pow99i = 0;
                                c_powr(_conj98r, _conj98i, k, &_pow99r, &_pow99i);
                                double _add100r = 0, _add100i = 0;
                                _add100r = _pow96r + _pow99r; _add100i = _pow96i + _pow99i;
                                { int _idx = j; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add100r; cIm[_idx] = _add100i; } }
                            }
                        }
                    }
                }
            }
        }
    }
    double _c101r = 0, _c101i = 0;
    _c101r = 100.0; _c101i = 0;
    double _c102r = 0, _c102i = 0;
    _c102r = 3.0; _c102i = 0;
    double _pow103r = 0, _pow103i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow103r, &_pow103i);
    c_mul(_pow103r, _pow103i, x1r, x1i, &_pow103r, &_pow103i);
    double _mul104r = 0, _mul104i = 0;
    c_mul(_c101r, _c101i, _pow103r, _pow103i, &_mul104r, &_mul104i);
    double _c105r = 0, _c105i = 0;
    _c105r = 0.0; _c105i = 50.0;
    double _c106r = 0, _c106i = 0;
    _c106r = 2.0; _c106i = 0;
    double _pow107r = 0, _pow107i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow107r, &_pow107i);
    double _mul108r = 0, _mul108i = 0;
    c_mul(_c105r, _c105i, _pow107r, _pow107i, &_mul108r, &_mul108i);
    double _sub109r = 0, _sub109i = 0;
    _sub109r = _mul104r - _mul108r; _sub109i = _mul104i - _mul108i;
    double _c110r = 0, _c110i = 0;
    _c110r = 25.0; _c110i = 0;
    double _mul111r = 0, _mul111i = 0;
    c_mul(_c110r, _c110i, x1r, x1i, &_mul111r, &_mul111i);
    double _mul112r = 0, _mul112i = 0;
    c_mul(_mul111r, _mul111i, x2r, x2i, &_mul112r, &_mul112i);
    double _add113r = 0, _add113i = 0;
    _add113r = _sub109r + _mul112r; _add113i = _sub109i + _mul112i;
    { int _idx = 11; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add113r; cIm[_idx] = _add113i; } }
    double _c114r = 0, _c114i = 0;
    _c114r = 0.0; _c114i = 200.0;
    double _sin115r = 0, _sin115i = 0;
    c_sin(x1r, x1i, &_sin115r, &_sin115i);
    double _mul116r = 0, _mul116i = 0;
    c_mul(_c114r, _c114i, _sin115r, _sin115i, &_mul116r, &_mul116i);
    double _c117r = 0, _c117i = 0;
    _c117r = 150.0; _c117i = 0;
    double _cos118r = 0, _cos118i = 0;
    c_cos(x2r, x2i, &_cos118r, &_cos118i);
    double _mul119r = 0, _mul119i = 0;
    c_mul(_c117r, _c117i, _cos118r, _cos118i, &_mul119r, &_mul119i);
    double _add120r = 0, _add120i = 0;
    _add120r = _mul116r + _mul119r; _add120i = _mul116i + _mul119i;
    { int _idx = 17; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add120r; cIm[_idx] = _add120i; } }
    double _c121r = 0, _c121i = 0;
    _c121r = 300.0; _c121i = 0;
    double _abs122r = 0, _abs122i = 0;
    _abs122r = c_abs(x1r, x1i); _abs122i = 0;
    double _c123r = 0, _c123i = 0;
    _c123r = 1.0; _c123i = 0;
    double _add124r = 0, _add124i = 0;
    _add124r = _abs122r + _c123r; _add124i = _abs122i + _c123i;
    double _log125r = 0, _log125i = 0;
    c_log(_add124r, _add124i, &_log125r, &_log125i);
    double _mul126r = 0, _mul126i = 0;
    c_mul(_c121r, _c121i, _log125r, _log125i, &_mul126r, &_mul126i);
    double _c127r = 0, _c127i = 0;
    _c127r = 0.0; _c127i = 100.0;
    double _abs128r = 0, _abs128i = 0;
    _abs128r = c_abs(x2r, x2i); _abs128i = 0;
    double _c129r = 0, _c129i = 0;
    _c129r = 2.0; _c129i = 0;
    double _pow130r = 0, _pow130i = 0;
    c_mul(_abs128r, _abs128i, _abs128r, _abs128i, &_pow130r, &_pow130i);
    double _mul131r = 0, _mul131i = 0;
    c_mul(_c127r, _c127i, _pow130r, _pow130i, &_mul131r, &_mul131i);
    double _add132r = 0, _add132i = 0;
    _add132r = _mul126r + _mul131r; _add132i = _mul126i + _mul131i;
    { int _idx = 26; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add132r; cIm[_idx] = _add132i; } }
    double _c133r = 0, _c133i = 0;
    _c133r = 400.0; _c133i = 0;
    double _mul134r = 0, _mul134i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul134r, &_mul134i);
    double _re135r = 0, _re135i = 0;
    _re135r = _mul134r; _re135i = 0;
    double _mul136r = 0, _mul136i = 0;
    c_mul(_c133r, _c133i, _re135r, _re135i, &_mul136r, &_mul136i);
    double _c137r = 0, _c137i = 0;
    _c137r = 0.0; _c137i = 200.0;
    double _add138r = 0, _add138i = 0;
    _add138r = x1r + x2r; _add138i = x1i + x2i;
    double _im139r = 0, _im139i = 0;
    _im139r = _add138i; _im139i = 0;
    double _mul140r = 0, _mul140i = 0;
    c_mul(_c137r, _c137i, _im139r, _im139i, &_mul140r, &_mul140i);
    double _sub141r = 0, _sub141i = 0;
    _sub141r = _mul136r - _mul140r; _sub141i = _mul136i - _mul140i;
    { int _idx = 33; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub141r; cIm[_idx] = _sub141i; } }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_278: auto-stubbed (unhandled constructs in source) */
static void poly_278_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_279: auto-stubbed (unhandled constructs in source) */
static void poly_279_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_280: auto-stubbed (unhandled constructs in source) */
static void poly_280_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_281: auto-stubbed (unhandled constructs in source) */
static void poly_281_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_282_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 2.5; _c1i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _c1r; cIm[_idx] = _c1i; } }
    double _c2r = 0, _c2i = 0;
    _c2r = 4.2; _c2i = 0;
    double _neg3r = 0, _neg3i = 0;
    _neg3r = -(_c2r); _neg3i = -(_c2i);
    { int _idx = 6; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _neg3r; cIm[_idx] = _neg3i; } }
    double _c4r = 0, _c4i = 0;
    _c4r = 3.8; _c4i = 0;
    { int _idx = 13; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _c4r; cIm[_idx] = _c4i; } }
    double _c5r = 0, _c5i = 0;
    _c5r = 16.5; _c5i = 0;
    double _neg6r = 0, _neg6i = 0;
    _neg6r = -(_c5r); _neg6i = -(_c5i);
    { int _idx = 20; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _neg6r; cIm[_idx] = _neg6i; } }
    double _c7r = 0, _c7i = 0;
    _c7r = 5.3; _c7i = 0;
    { int _idx = 27; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _c7r; cIm[_idx] = _c7i; } }
    double _c8r = 0, _c8i = 0;
    _c8r = 0.6; _c8i = 0;
    { int _idx = 34; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _c8r; cIm[_idx] = _c8i; } }
    for (int j = 2; j < 35; j++) {
        double k = 0;
        double r = 0;
        double _c9r = 0, _c9i = 0;
        _c9r = 4.0; _c9i = 0;
        double _mod10r = 0, _mod10i = 0;
        _mod10r = fmod(j, _c9r); _mod10i = 0;
        double _c11r = 0, _c11i = 0;
        _c11r = 0.0; _c11i = 0;
        if (_mod10r == _c11r) {
            double _c12r = 0, _c12i = 0;
            _c12r = 2.0; _c12i = 0;
            double _fdiv13r = 0, _fdiv13i = 0;
            c_div(j, 0, _c12r, _c12i, &_fdiv13r, &_fdiv13i);
            _fdiv13r = floor(_fdiv13r); _fdiv13i = 0;
            k = _fdiv13r;
            double _c14r = 0, _c14i = 0;
            _c14r = 0.0; _c14i = 150.0;
            double _pow15r = 0, _pow15i = 0;
            c_powr(x1r, x1i, k, &_pow15r, &_pow15i);
            double _mul16r = 0, _mul16i = 0;
            c_mul(_c14r, _c14i, _pow15r, _pow15i, &_mul16r, &_mul16i);
            double _c17r = 0, _c17i = 0;
            _c17r = 75.0; _c17i = 0;
            double _conj18r = 0, _conj18i = 0;
            _conj18r = x2r; _conj18i = -(x2i);
            double _mul19r = 0, _mul19i = 0;
            c_mul(_c17r, _c17i, _conj18r, _conj18i, &_mul19r, &_mul19i);
            double _add20r = 0, _add20i = 0;
            _add20r = _mul16r + _mul19r; _add20i = _mul16i + _mul19i;
            double _ang21r = 0, _ang21i = 0;
            _ang21r = c_arg(x1r, x1i); _ang21i = 0;
            double _mul22r = 0, _mul22i = 0;
            c_mul(k, 0, _ang21r, _ang21i, &_mul22r, &_mul22i);
            double _sin23r = 0, _sin23i = 0;
            c_sin(_mul22r, _mul22i, &_sin23r, &_sin23i);
            double _mul24r = 0, _mul24i = 0;
            c_mul(_add20r, _add20i, _sin23r, _sin23i, &_mul24r, &_mul24i);
            double _c25r = 0, _c25i = 0;
            _c25r = 50.0; _c25i = 0;
            double _abs26r = 0, _abs26i = 0;
            _abs26r = c_abs(x2r, x2i); _abs26i = 0;
            double _c27r = 0, _c27i = 0;
            _c27r = 1.0; _c27i = 0;
            double _add28r = 0, _add28i = 0;
            _add28r = _abs26r + _c27r; _add28i = _abs26i + _c27i;
            double _log29r = 0, _log29i = 0;
            c_log(_add28r, _add28i, &_log29r, &_log29i);
            double _mul30r = 0, _mul30i = 0;
            c_mul(_c25r, _c25i, _log29r, _log29i, &_mul30r, &_mul30i);
            double _sub31r = 0, _sub31i = 0;
            _sub31r = _mul24r - _mul30r; _sub31i = _mul24i - _mul30i;
            { int _idx = j; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub31r; cIm[_idx] = _sub31i; } }
        } else {
            double _c32r = 0, _c32i = 0;
            _c32r = 3.0; _c32i = 0;
            double _mod33r = 0, _mod33i = 0;
            _mod33r = fmod(j, _c32r); _mod33i = 0;
            double _c34r = 0, _c34i = 0;
            _c34r = 0.0; _c34i = 0;
            if (_mod33r == _c34r) {
                double _c35r = 0, _c35i = 0;
                _c35r = 5.0; _c35i = 0;
                double _mod36r = 0, _mod36i = 0;
                _mod36r = fmod(j, _c35r); _mod36i = 0;
                k = _mod36r;
                double _c37r = 0, _c37i = 0;
                _c37r = 200.0; _c37i = 0;
                double _pow38r = 0, _pow38i = 0;
                c_powr(x2r, x2i, k, &_pow38r, &_pow38i);
                double _mul39r = 0, _mul39i = 0;
                c_mul(x1r, x1i, _pow38r, _pow38i, &_mul39r, &_mul39i);
                double _re40r = 0, _re40i = 0;
                _re40r = _mul39r; _re40i = 0;
                double _mul41r = 0, _mul41i = 0;
                c_mul(_c37r, _c37i, _re40r, _re40i, &_mul41r, &_mul41i);
                double _c42r = 0, _c42i = 0;
                _c42r = 0.0; _c42i = 100.0;
                double _sub43r = 0, _sub43i = 0;
                _sub43r = x1r - x2r; _sub43i = x1i - x2i;
                double _im44r = 0, _im44i = 0;
                _im44r = _sub43i; _im44i = 0;
                double _mul45r = 0, _mul45i = 0;
                c_mul(_c42r, _c42i, _im44r, _im44i, &_mul45r, &_mul45i);
                double _add46r = 0, _add46i = 0;
                _add46r = _mul41r + _mul45r; _add46i = _mul41i + _mul45i;
                double _ang47r = 0, _ang47i = 0;
                _ang47r = c_arg(x2r, x2i); _ang47i = 0;
                double _mul48r = 0, _mul48i = 0;
                c_mul(k, 0, _ang47r, _ang47i, &_mul48r, &_mul48i);
                double _cos49r = 0, _cos49i = 0;
                c_cos(_mul48r, _mul48i, &_cos49r, &_cos49i);
                double _mul50r = 0, _mul50i = 0;
                c_mul(_add46r, _add46i, _cos49r, _cos49i, &_mul50r, &_mul50i);
                { int _idx = j; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul50r; cIm[_idx] = _mul50i; } }
            } else {
                double _c51r = 0, _c51i = 0;
                _c51r = 7.0; _c51i = 0;
                double _mod52r = 0, _mod52i = 0;
                _mod52r = fmod(j, _c51r); _mod52i = 0;
                r = _mod52r;
                double _conj53r = 0, _conj53i = 0;
                _conj53r = x1r; _conj53i = -(x1i);
                double _pow54r = 0, _pow54i = 0;
                c_powr(_conj53r, _conj53i, r, &_pow54r, &_pow54i);
                double _pow55r = 0, _pow55i = 0;
                c_powr(x2r, x2i, j, &_pow55r, &_pow55i);
                double _mul56r = 0, _mul56i = 0;
                c_mul(_pow54r, _pow54i, _pow55r, _pow55i, &_mul56r, &_mul56i);
                double _pow57r = 0, _pow57i = 0;
                c_powr(x1r, x1i, j, &_pow57r, &_pow57i);
                double _abs58r = 0, _abs58i = 0;
                _abs58r = c_abs(_pow57r, _pow57i); _abs58i = 0;
                double _pow59r = 0, _pow59i = 0;
                c_powr(x2r, x2i, r, &_pow59r, &_pow59i);
                double _abs60r = 0, _abs60i = 0;
                _abs60r = c_abs(_pow59r, _pow59i); _abs60i = 0;
                double _mul61r = 0, _mul61i = 0;
                c_mul(_abs58r, _abs58i, _abs60r, _abs60i, &_mul61r, &_mul61i);
                double _add62r = 0, _add62i = 0;
                _add62r = _mul56r + _mul61r; _add62i = _mul56i + _mul61i;
                { int _idx = j; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add62r; cIm[_idx] = _add62i; } }
            }
        }
    }
    double _c63r = 0, _c63i = 0;
    _c63r = 0.0; _c63i = 180.0;
    double _c64r = 0, _c64i = 0;
    _c64r = 3.0; _c64i = 0;
    double _pow65r = 0, _pow65i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow65r, &_pow65i);
    c_mul(_pow65r, _pow65i, x1r, x1i, &_pow65r, &_pow65i);
    double _mul66r = 0, _mul66i = 0;
    c_mul(_c63r, _c63i, _pow65r, _pow65i, &_mul66r, &_mul66i);
    double _c67r = 0, _c67i = 0;
    _c67r = 120.0; _c67i = 0;
    double _c68r = 0, _c68i = 0;
    _c68r = 2.0; _c68i = 0;
    double _pow69r = 0, _pow69i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow69r, &_pow69i);
    double _mul70r = 0, _mul70i = 0;
    c_mul(_c67r, _c67i, _pow69r, _pow69i, &_mul70r, &_mul70i);
    double _sub71r = 0, _sub71i = 0;
    _sub71r = _mul66r - _mul70r; _sub71i = _mul66i - _mul70i;
    double _c72r = 0, _c72i = 0;
    _c72r = 90.0; _c72i = 0;
    double _sin73r = 0, _sin73i = 0;
    c_sin(x1r, x1i, &_sin73r, &_sin73i);
    double _mul74r = 0, _mul74i = 0;
    c_mul(_c72r, _c72i, _sin73r, _sin73i, &_mul74r, &_mul74i);
    double _cos75r = 0, _cos75i = 0;
    c_cos(x2r, x2i, &_cos75r, &_cos75i);
    double _mul76r = 0, _mul76i = 0;
    c_mul(_mul74r, _mul74i, _cos75r, _cos75i, &_mul76r, &_mul76i);
    double _add77r = 0, _add77i = 0;
    _add77r = _sub71r + _mul76r; _add77i = _sub71i + _mul76i;
    { int _idx = 9; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add77r; cIm[_idx] = _add77i; } }
    double _c78r = 0, _c78i = 0;
    _c78r = 0.0; _c78i = 220.0;
    double _c79r = 0, _c79i = 0;
    _c79r = 4.0; _c79i = 0;
    double _pow80r = 0, _pow80i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow80r, &_pow80i);
    c_mul(_pow80r, _pow80i, _pow80r, _pow80i, &_pow80r, &_pow80i);
    double _mul81r = 0, _mul81i = 0;
    c_mul(_c78r, _c78i, _pow80r, _pow80i, &_mul81r, &_mul81i);
    double _c82r = 0, _c82i = 0;
    _c82r = 130.0; _c82i = 0;
    double _c83r = 0, _c83i = 0;
    _c83r = 3.0; _c83i = 0;
    double _pow84r = 0, _pow84i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow84r, &_pow84i);
    c_mul(_pow84r, _pow84i, x1r, x1i, &_pow84r, &_pow84i);
    double _re85r = 0, _re85i = 0;
    _re85r = _pow84r; _re85i = 0;
    double _mul86r = 0, _mul86i = 0;
    c_mul(_c82r, _c82i, _re85r, _re85i, &_mul86r, &_mul86i);
    double _add87r = 0, _add87i = 0;
    _add87r = _mul81r + _mul86r; _add87i = _mul81i + _mul86i;
    double _c88r = 0, _c88i = 0;
    _c88r = 100.0; _c88i = 0;
    double _im89r = 0, _im89i = 0;
    _im89r = x2i; _im89i = 0;
    double _mul90r = 0, _mul90i = 0;
    c_mul(_c88r, _c88i, _im89r, _im89i, &_mul90r, &_mul90i);
    double _sub91r = 0, _sub91i = 0;
    _sub91r = _add87r - _mul90r; _sub91i = _add87i - _mul90i;
    { int _idx = 19; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub91r; cIm[_idx] = _sub91i; } }
    double _c92r = 0, _c92i = 0;
    _c92r = 0.0; _c92i = 260.0;
    double _c93r = 0, _c93i = 0;
    _c93r = 2.0; _c93i = 0;
    double _pow94r = 0, _pow94i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow94r, &_pow94i);
    double _mul95r = 0, _mul95i = 0;
    c_mul(_c92r, _c92i, _pow94r, _pow94i, &_mul95r, &_mul95i);
    double _mul96r = 0, _mul96i = 0;
    c_mul(_mul95r, _mul95i, x2r, x2i, &_mul96r, &_mul96i);
    double _c97r = 0, _c97i = 0;
    _c97r = 160.0; _c97i = 0;
    double _mul98r = 0, _mul98i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul98r, &_mul98i);
    double _abs99r = 0, _abs99i = 0;
    _abs99r = c_abs(_mul98r, _mul98i); _abs99i = 0;
    double _c100r = 0, _c100i = 0;
    _c100r = 1.0; _c100i = 0;
    double _add101r = 0, _add101i = 0;
    _add101r = _abs99r + _c100r; _add101i = _abs99i + _c100i;
    double _log102r = 0, _log102i = 0;
    c_log(_add101r, _add101i, &_log102r, &_log102i);
    double _mul103r = 0, _mul103i = 0;
    c_mul(_c97r, _c97i, _log102r, _log102i, &_mul103r, &_mul103i);
    double _add104r = 0, _add104i = 0;
    _add104r = _mul96r + _mul103r; _add104i = _mul96i + _mul103i;
    double _c105r = 0, _c105i = 0;
    _c105r = 110.0; _c105i = 0;
    double _conj106r = 0, _conj106i = 0;
    _conj106r = x1r; _conj106i = -(x1i);
    double _mul107r = 0, _mul107i = 0;
    c_mul(_c105r, _c105i, _conj106r, _conj106i, &_mul107r, &_mul107i);
    double _sub108r = 0, _sub108i = 0;
    _sub108r = _add104r - _mul107r; _sub108i = _add104i - _mul107i;
    { int _idx = 29; if (_idx >= 0 && _idx < 35) { cRe[_idx] = _sub108r; cIm[_idx] = _sub108i; } }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_283: auto-stubbed (unhandled constructs in source) */
static void poly_283_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_284_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
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
        double _cos7r = 0, _cos7i = 0;
        c_cos(j, 0, &_cos7r, &_cos7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(_ang6r, _ang6i, _cos7r, _cos7i, &_mul8r, &_mul8i);
        double _add9r = 0, _add9i = 0;
        _add9r = _mul5r + _mul8r; _add9i = _mul5i + _mul8i;
        double phase_r = _add9r, phase_i = _add9i;
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(x1r, x1i); _abs10i = 0;
        double _c11r = 0, _c11i = 0;
        _c11r = 5.0; _c11i = 0;
        double _mod12r = 0, _mod12i = 0;
        _mod12r = fmod(j, _c11r); _mod12i = 0;
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = _mod12r + _c13r; _add14i = _mod12i + _c13i;
        double _pow15r = 0, _pow15i = 0;
        c_powc(_abs10r, _abs10i, _add14r, _add14i, &_pow15r, &_pow15i);
        double _abs16r = 0, _abs16i = 0;
        _abs16r = c_abs(x2r, x2i); _abs16i = 0;
        double _c17r = 0, _c17i = 0;
        _c17r = 7.0; _c17i = 0;
        double _fdiv18r = 0, _fdiv18i = 0;
        c_div(j, 0, _c17r, _c17i, &_fdiv18r, &_fdiv18i);
        _fdiv18r = floor(_fdiv18r); _fdiv18i = 0;
        double _c19r = 0, _c19i = 0;
        _c19r = 1.0; _c19i = 0;
        double _add20r = 0, _add20i = 0;
        _add20r = _fdiv18r + _c19r; _add20i = _fdiv18i + _c19i;
        double _pow21r = 0, _pow21i = 0;
        c_powc(_abs16r, _abs16i, _add20r, _add20i, &_pow21r, &_pow21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _pow15r + _pow21r; _add22i = _pow15i + _pow21i;
        double magnitude_r = _add22r, magnitude_i = _add22i;
        double _add23r = 0, _add23i = 0;
        _add23r = x1r + x2r; _add23i = x1i + x2i;
        double _abs24r = 0, _abs24i = 0;
        _abs24r = c_abs(_add23r, _add23i); _abs24i = 0;
        double _c25r = 0, _c25i = 0;
        _c25r = 1.0; _c25i = 0;
        double _add26r = 0, _add26i = 0;
        _add26r = _abs24r + _c25r; _add26i = _abs24i + _c25i;
        double _log27r = 0, _log27i = 0;
        c_log(_add26r, _add26i, &_log27r, &_log27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(j, 0, M_PI, 0, &_mul28r, &_mul28i);
        double _c29r = 0, _c29i = 0;
        _c29r = 3.0; _c29i = 0;
        double _div30r = 0, _div30i = 0;
        c_div(_mul28r, _mul28i, _c29r, _c29i, &_div30r, &_div30i);
        double _cos31r = 0, _cos31i = 0;
        c_cos(_div30r, _div30i, &_cos31r, &_cos31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_log27r, _log27i, _cos31r, _cos31i, &_mul32r, &_mul32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(j, 0, M_PI, 0, &_mul33r, &_mul33i);
        double _c34r = 0, _c34i = 0;
        _c34r = 4.0; _c34i = 0;
        double _div35r = 0, _div35i = 0;
        c_div(_mul33r, _mul33i, _c34r, _c34i, &_div35r, &_div35i);
        double _sin36r = 0, _sin36i = 0;
        c_sin(_div35r, _div35i, &_sin36r, &_sin36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _mul32r + _sin36r; _add37i = _mul32i + _sin36i;
        double perturb_r = _add37r, perturb_i = _add37i;
        double _c38r = 0, _c38i = 0;
        _c38r = 0.0; _c38i = 1.0;
        double _mul39r = 0, _mul39i = 0;
        c_mul(_c38r, _c38i, phase_r, phase_i, &_mul39r, &_mul39i);
        double _exp40r = 0, _exp40i = 0;
        c_exp2(_mul39r, _mul39i, &_exp40r, &_exp40i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(magnitude_r, magnitude_i, _exp40r, _exp40i, &_mul41r, &_mul41i);
        double _add42r = 0, _add42i = 0;
        _add42r = _mul41r + perturb_r; _add42i = _mul41i + perturb_i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add42r; cIm[_idx] = _add42i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_285_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _abs3r = 0, _abs3i = 0;
        _abs3r = c_abs(real_seq, 0); _abs3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 1.0; _c4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = _abs3r + _c4r; _add5i = _abs3i + _c4i;
        double _log6r = 0, _log6i = 0;
        c_log(_add5r, _add5i, &_log6r, &_log6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(real_seq, 0, j, 0, &_mul7r, &_mul7i);
        double _sin8r = 0, _sin8i = 0;
        c_sin(_mul7r, _mul7i, &_sin8r, &_sin8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(_log6r, _log6i, _sin8r, _sin8i, &_mul9r, &_mul9i);
        double _c10r = 0, _c10i = 0;
        _c10r = 1.0; _c10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = imag_seq + _c10r; _add11i = 0 + _c10i;
        double _add12r = 0, _add12i = 0;
        _add12r = _mul9r + _add11r; _add12i = _mul9i + _add11i;
        double mag_component_r = _add12r, mag_component_i = _add12i;
        double _mul13r = 0, _mul13i = 0;
        c_mul(imag_seq, 0, j, 0, &_mul13r, &_mul13i);
        double _cos14r = 0, _cos14i = 0;
        c_cos(_mul13r, _mul13i, &_cos14r, &_cos14i);
        double _c15r = 0, _c15i = 0;
        _c15r = 1.0; _c15i = 0;
        double _add16r = 0, _add16i = 0;
        _add16r = j + _c15r; _add16i = 0 + _c15i;
        double _div17r = 0, _div17i = 0;
        c_div(real_seq, 0, _add16r, _add16i, &_div17r, &_div17i);
        double _sin18r = 0, _sin18i = 0;
        c_sin(_div17r, _div17i, &_sin18r, &_sin18i);
        double _sub19r = 0, _sub19i = 0;
        _sub19r = _cos14r - _sin18r; _sub19i = _cos14i - _sin18i;
        double angle_component_r = _sub19r, angle_component_i = _sub19i;
        double _cos20r = 0, _cos20i = 0;
        c_cos(angle_component_r, angle_component_i, &_cos20r, &_cos20i);
        double _c21r = 0, _c21i = 0;
        _c21r = 0.0; _c21i = 1.0;
        double _sin22r = 0, _sin22i = 0;
        c_sin(angle_component_r, angle_component_i, &_sin22r, &_sin22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_c21r, _c21i, _sin22r, _sin22i, &_mul23r, &_mul23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _cos20r + _mul23r; _add24i = _cos20i + _mul23i;
        double _mul25r = 0, _mul25i = 0;
        c_mul(mag_component_r, mag_component_i, _add24r, _add24i, &_mul25r, &_mul25i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul25r; cIm[_idx] = _mul25i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_286_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int k = 1; k < (int)(_add2r); k++) {
        double _re3r = 0, _re3i = 0;
        _re3r = x1r; _re3i = 0;
        double _re4r = 0, _re4i = 0;
        _re4r = x2r; _re4i = 0;
        double _mul5r = 0, _mul5i = 0;
        c_mul(k, 0, _re4r, _re4i, &_mul5r, &_mul5i);
        double _sin6r = 0, _sin6i = 0;
        c_sin(_mul5r, _mul5i, &_sin6r, &_sin6i);
        double _mul7r = 0, _mul7i = 0;
        c_mul(_re3r, _re3i, _sin6r, _sin6i, &_mul7r, &_mul7i);
        double _im8r = 0, _im8i = 0;
        _im8r = x1i; _im8i = 0;
        double _im9r = 0, _im9i = 0;
        _im9r = x2i; _im9i = 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(k, 0, _im9r, _im9i, &_mul10r, &_mul10i);
        double _cos11r = 0, _cos11i = 0;
        c_cos(_mul10r, _mul10i, &_cos11r, &_cos11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_im8r, _im8i, _cos11r, _cos11i, &_mul12r, &_mul12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _mul7r + _mul12r; _add13i = _mul7i + _mul12i;
        double a_r = _add13r, a_i = _add13i;
        double _abs14r = 0, _abs14i = 0;
        _abs14r = c_abs(x1r, x1i); _abs14i = 0;
        double _c15r = 0, _c15i = 0;
        _c15r = 1.0; _c15i = 0;
        double _add16r = 0, _add16i = 0;
        _add16r = _abs14r + _c15r; _add16i = _abs14i + _c15i;
        double _log17r = 0, _log17i = 0;
        c_log(_add16r, _add16i, &_log17r, &_log17i);
        double _ang18r = 0, _ang18i = 0;
        _ang18r = c_arg(x2r, x2i); _ang18i = 0;
        double _mul19r = 0, _mul19i = 0;
        c_mul(k, 0, _ang18r, _ang18i, &_mul19r, &_mul19i);
        double _c20r = 0, _c20i = 0;
        _c20r = 1.0; _c20i = 0;
        double _add21r = 0, _add21i = 0;
        _add21r = k + _c20r; _add21i = 0 + _c20i;
        double _div22r = 0, _div22i = 0;
        c_div(_mul19r, _mul19i, _add21r, _add21i, &_div22r, &_div22i);
        double _sin23r = 0, _sin23i = 0;
        c_sin(_div22r, _div22i, &_sin23r, &_sin23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_log17r, _log17i, _sin23r, _sin23i, &_mul24r, &_mul24i);
        double b_r = _mul24r, b_i = _mul24i;
        double _abs25r = 0, _abs25i = 0;
        _abs25r = c_abs(x2r, x2i); _abs25i = 0;
        double _pow26r = 0, _pow26i = 0;
        c_powr(_abs25r, _abs25i, k, &_pow26r, &_pow26i);
        double _re27r = 0, _re27i = 0;
        _re27r = x1r; _re27i = 0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(k, 0, _re27r, _re27i, &_mul28r, &_mul28i);
        double _cos29r = 0, _cos29i = 0;
        c_cos(_mul28r, _mul28i, &_cos29r, &_cos29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_pow26r, _pow26i, _cos29r, _cos29i, &_mul30r, &_mul30i);
        double c_r = _mul30r, c_i = _mul30i;
        double _im31r = 0, _im31i = 0;
        _im31r = x1i; _im31i = 0;
        double _mul32r = 0, _mul32i = 0;
        c_mul(k, 0, _im31r, _im31i, &_mul32r, &_mul32i);
        double _sin33r = 0, _sin33i = 0;
        c_sin(_mul32r, _mul32i, &_sin33r, &_sin33i);
        double _re34r = 0, _re34i = 0;
        _re34r = x2r; _re34i = 0;
        double _mul35r = 0, _mul35i = 0;
        c_mul(k, 0, _re34r, _re34i, &_mul35r, &_mul35i);
        double _cos36r = 0, _cos36i = 0;
        c_cos(_mul35r, _mul35i, &_cos36r, &_cos36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _sin33r + _cos36r; _add37i = _sin33i + _cos36i;
        double d_r = _add37r, d_i = _add37i;
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
        double angle_r = _add44r, angle_i = _add44i;
        double _add45r = 0, _add45i = 0;
        _add45r = a_r + b_r; _add45i = a_i + b_i;
        double _add46r = 0, _add46i = 0;
        _add46r = _add45r + c_r; _add46i = _add45i + c_i;
        double _add47r = 0, _add47i = 0;
        _add47r = _add46r + d_r; _add47i = _add46i + d_i;
        double magnitude_r = _add47r, magnitude_i = _add47i;
        double _c48r = 0, _c48i = 0;
        _c48r = 0.0; _c48i = 1.0;
        double _mul49r = 0, _mul49i = 0;
        c_mul(_c48r, _c48i, angle_r, angle_i, &_mul49r, &_mul49i);
        double _exp50r = 0, _exp50i = 0;
        c_exp2(_mul49r, _mul49i, &_exp50r, &_exp50i);
        double _mul51r = 0, _mul51i = 0;
        c_mul(magnitude_r, magnitude_i, _exp50r, _exp50i, &_mul51r, &_mul51i);
        double _conj52r = 0, _conj52i = 0;
        _conj52r = x1r; _conj52i = -(x1i);
        double _conj53r = 0, _conj53i = 0;
        _conj53r = x2r; _conj53i = -(x2i);
        double _pow54r = 0, _pow54i = 0;
        c_powr(_conj53r, _conj53i, k, &_pow54r, &_pow54i);
        double _mul55r = 0, _mul55i = 0;
        c_mul(_conj52r, _conj52i, _pow54r, _pow54i, &_mul55r, &_mul55i);
        double _add56r = 0, _add56i = 0;
        _add56r = _mul51r + _mul55r; _add56i = _mul51i + _mul55i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add56r; cIm[_idx] = _add56i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_287_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
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
        double _arr7r = 0, _arr7i = 0;
        { int _idx = (j - 1); _arr7r = (_idx >= 0 && _idx < 35) ? rec_seq[_idx] : 0.0; _arr7i = 0; }
        double r_r = _arr7r, r_i = _arr7i;
        double _arr8r = 0, _arr8i = 0;
        { int _idx = (j - 1); _arr8r = (_idx >= 0 && _idx < 35) ? imc_seq[_idx] : 0.0; _arr8i = 0; }
        double m_r = _arr8r, m_i = _arr8i;
        double _mul9r = 0, _mul9i = 0;
        c_mul(r_r, r_i, m_r, m_i, &_mul9r, &_mul9i);
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(_mul9r, _mul9i); _abs10i = 0;
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = _abs10r + _c11r; _add12i = _abs10i + _c11i;
        double _log13r = 0, _log13i = 0;
        c_log(_add12r, _add12i, &_log13r, &_log13i);
        double _c14r = 0, _c14i = 0;
        _c14r = 2.0; _c14i = 0;
        double _pow15r = 0, _pow15i = 0;
        c_mul(j, 0, j, 0, &_pow15r, &_pow15i);
        double _sin16r = 0, _sin16i = 0;
        c_sin(j, 0, &_sin16r, &_sin16i);
        double _cos17r = 0, _cos17i = 0;
        c_cos(j, 0, &_cos17r, &_cos17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_sin16r, _sin16i, _cos17r, _cos17i, &_mul18r, &_mul18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _pow15r + _mul18r; _add19i = _pow15i + _mul18i;
        double _mul20r = 0, _mul20i = 0;
        c_mul(_log13r, _log13i, _add19r, _add19i, &_mul20r, &_mul20i);
        double mag_part_r = _mul20r, mag_part_i = _mul20i;
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
        _c27r = 4.0; _c27i = 0;
        double _div28r = 0, _div28i = 0;
        c_div(j, 0, _c27r, _c27i, &_div28r, &_div28i);
        double _cos29r = 0, _cos29i = 0;
        c_cos(_div28r, _div28i, &_cos29r, &_cos29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_ang26r, _ang26i, _cos29r, _cos29i, &_mul30r, &_mul30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _mul25r + _mul30r; _add31i = _mul25i + _mul30i;
        double _mul32r = 0, _mul32i = 0;
        c_mul(m_r, m_i, M_PI, 0, &_mul32r, &_mul32i);
        double _c33r = 0, _c33i = 0;
        _c33r = 5.0; _c33i = 0;
        double _div34r = 0, _div34i = 0;
        c_div(_mul32r, _mul32i, _c33r, _c33i, &_div34r, &_div34i);
        double _sin35r = 0, _sin35i = 0;
        c_sin(_div34r, _div34i, &_sin35r, &_sin35i);
        double _add36r = 0, _add36i = 0;
        _add36r = _add31r + _sin35r; _add36i = _add31i + _sin35i;
        double angle_part_r = _add36r, angle_part_i = _add36i;
        double _c37r = 0, _c37i = 0;
        _c37r = 0.0; _c37i = 1.0;
        double _mul38r = 0, _mul38i = 0;
        c_mul(_c37r, _c37i, angle_part_r, angle_part_i, &_mul38r, &_mul38i);
        double _exp39r = 0, _exp39i = 0;
        c_exp2(_mul38r, _mul38i, &_exp39r, &_exp39i);
        double _mul40r = 0, _mul40i = 0;
        c_mul(mag_part_r, mag_part_i, _exp39r, _exp39i, &_mul40r, &_mul40i);
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
        double coeff_r = _add47r, coeff_i = _add47i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = coeff_r; cIm[_idx] = coeff_i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_288: auto-stubbed (unhandled constructs in source) */
static void poly_288_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_289_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
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
        double mag_factor_r = _mul22r, mag_factor_i = _mul22i;
        double _arr23r = 0, _arr23i = 0;
        { int _idx = (j - 1); _arr23r = (_idx >= 0 && _idx < 35) ? rec_seq[_idx] : 0.0; _arr23i = 0; }
        double _c24r = 0, _c24i = 0;
        _c24r = 0.0; _c24i = 1.0;
        double _arr25r = 0, _arr25i = 0;
        { int _idx = (j - 1); _arr25r = (_idx >= 0 && _idx < 35) ? imc_seq[_idx] : 0.0; _arr25i = 0; }
        double _mul26r = 0, _mul26i = 0;
        c_mul(_c24r, _c24i, _arr25r, _arr25i, &_mul26r, &_mul26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _arr23r + _mul26r; _add27i = _arr23i + _mul26i;
        double _ang28r = 0, _ang28i = 0;
        _ang28r = c_arg(_add27r, _add27i); _ang28i = 0;
        double _mul29r = 0, _mul29i = 0;
        c_mul(j, 0, M_PI, 0, &_mul29r, &_mul29i);
        double _c30r = 0, _c30i = 0;
        _c30r = 3.0; _c30i = 0;
        double _div31r = 0, _div31i = 0;
        c_div(_mul29r, _mul29i, _c30r, _c30i, &_div31r, &_div31i);
        double _cos32r = 0, _cos32i = 0;
        c_cos(_div31r, _div31i, &_cos32r, &_cos32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(j, 0, M_PI, 0, &_mul33r, &_mul33i);
        double _c34r = 0, _c34i = 0;
        _c34r = 5.0; _c34i = 0;
        double _div35r = 0, _div35i = 0;
        c_div(_mul33r, _mul33i, _c34r, _c34i, &_div35r, &_div35i);
        double _sin36r = 0, _sin36i = 0;
        c_sin(_div35r, _div35i, &_sin36r, &_sin36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(_cos32r, _cos32i, _sin36r, _sin36i, &_mul37r, &_mul37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _ang28r + _mul37r; _add38i = _ang28i + _mul37i;
        double angle_factor_r = _add38r, angle_factor_i = _add38i;
        double _cos39r = 0, _cos39i = 0;
        c_cos(angle_factor_r, angle_factor_i, &_cos39r, &_cos39i);
        double _c40r = 0, _c40i = 0;
        _c40r = 0.0; _c40i = 1.0;
        double _sin41r = 0, _sin41i = 0;
        c_sin(angle_factor_r, angle_factor_i, &_sin41r, &_sin41i);
        double _mul42r = 0, _mul42i = 0;
        c_mul(_c40r, _c40i, _sin41r, _sin41i, &_mul42r, &_mul42i);
        double _add43r = 0, _add43i = 0;
        _add43r = _cos39r + _mul42r; _add43i = _cos39i + _mul42i;
        double _mul44r = 0, _mul44i = 0;
        c_mul(mag_factor_r, mag_factor_i, _add43r, _add43i, &_mul44r, &_mul44i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul44r; cIm[_idx] = _mul44i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_290_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 34.0;
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
        double angle = 0;
        double mag = 0;
        double _c5r = 0, _c5i = 0;
        _c5r = 4.0; _c5i = 0;
        double _mod6r = 0, _mod6i = 0;
        _mod6r = fmod(j, _c5r); _mod6i = 0;
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        if (_mod6r == _c7r) {
            double _abs8r = 0, _abs8i = 0;
            _abs8r = c_abs(x1r, x1i); _abs8i = 0;
            double _c9r = 0, _c9i = 0;
            _c9r = 2.0; _c9i = 0;
            double _pow10r = 0, _pow10i = 0;
            c_mul(j, 0, j, 0, &_pow10r, &_pow10i);
            double _add11r = 0, _add11i = 0;
            _add11r = _abs8r + _pow10r; _add11i = _abs8i + _pow10i;
            double _log12r = 0, _log12i = 0;
            c_log(_add11r, _add11i, &_log12r, &_log12i);
            double _mul13r = 0, _mul13i = 0;
            c_mul(j, 0, M_PI, 0, &_mul13r, &_mul13i);
            double _c14r = 0, _c14i = 0;
            _c14r = 6.0; _c14i = 0;
            double _div15r = 0, _div15i = 0;
            c_div(_mul13r, _mul13i, _c14r, _c14i, &_div15r, &_div15i);
            double _sin16r = 0, _sin16i = 0;
            c_sin(_div15r, _div15i, &_sin16r, &_sin16i);
            double _mul17r = 0, _mul17i = 0;
            c_mul(j, 0, M_PI, 0, &_mul17r, &_mul17i);
            double _c18r = 0, _c18i = 0;
            _c18r = 4.0; _c18i = 0;
            double _div19r = 0, _div19i = 0;
            c_div(_mul17r, _mul17i, _c18r, _c18i, &_div19r, &_div19i);
            double _cos20r = 0, _cos20i = 0;
            c_cos(_div19r, _div19i, &_cos20r, &_cos20i);
            double _mul21r = 0, _mul21i = 0;
            c_mul(_sin16r, _sin16i, _cos20r, _cos20i, &_mul21r, &_mul21i);
            double _add22r = 0, _add22i = 0;
            _add22r = _log12r + _mul21r; _add22i = _log12i + _mul21i;
            mag = _add22r;
            double _ang23r = 0, _ang23i = 0;
            _ang23r = c_arg(x1r, x1i); _ang23i = 0;
            double _mul24r = 0, _mul24i = 0;
            c_mul(_ang23r, _ang23i, j, 0, &_mul24r, &_mul24i);
            double _mul25r = 0, _mul25i = 0;
            c_mul(j, 0, M_PI, 0, &_mul25r, &_mul25i);
            double _c26r = 0, _c26i = 0;
            _c26r = 5.0; _c26i = 0;
            double _div27r = 0, _div27i = 0;
            c_div(_mul25r, _mul25i, _c26r, _c26i, &_div27r, &_div27i);
            double _sin28r = 0, _sin28i = 0;
            c_sin(_div27r, _div27i, &_sin28r, &_sin28i);
            double _add29r = 0, _add29i = 0;
            _add29r = _mul24r + _sin28r; _add29i = _mul24i + _sin28i;
            double _mul30r = 0, _mul30i = 0;
            c_mul(j, 0, M_PI, 0, &_mul30r, &_mul30i);
            double _c31r = 0, _c31i = 0;
            _c31r = 3.0; _c31i = 0;
            double _div32r = 0, _div32i = 0;
            c_div(_mul30r, _mul30i, _c31r, _c31i, &_div32r, &_div32i);
            double _cos33r = 0, _cos33i = 0;
            c_cos(_div32r, _div32i, &_cos33r, &_cos33i);
            double _sub34r = 0, _sub34i = 0;
            _sub34r = _add29r - _cos33r; _sub34i = _add29i - _cos33i;
            angle = _sub34r;
        } else {
            double _c35r = 0, _c35i = 0;
            _c35r = 4.0; _c35i = 0;
            double _mod36r = 0, _mod36i = 0;
            _mod36r = fmod(j, _c35r); _mod36i = 0;
            double _c37r = 0, _c37i = 0;
            _c37r = 2.0; _c37i = 0;
            if (_mod36r == _c37r) {
                double _abs38r = 0, _abs38i = 0;
                _abs38r = c_abs(x2r, x2i); _abs38i = 0;
                double _add39r = 0, _add39i = 0;
                _add39r = _abs38r + j; _add39i = _abs38i + 0;
                double _log40r = 0, _log40i = 0;
                c_log(_add39r, _add39i, &_log40r, &_log40i);
                double _mul41r = 0, _mul41i = 0;
                c_mul(_log40r, _log40i, 0, 0, &_mul41r, &_mul41i);
                mag = _mul41r;
                double _ang42r = 0, _ang42i = 0;
                _ang42r = c_arg(x2r, x2i); _ang42i = 0;
                double _c43r = 0, _c43i = 0;
                _c43r = 1.0; _c43i = 0;
                double _add44r = 0, _add44i = 0;
                _add44r = j + _c43r; _add44i = 0 + _c43i;
                double _div45r = 0, _div45i = 0;
                c_div(_ang42r, _ang42i, _add44r, _add44i, &_div45r, &_div45i);
                double _mul46r = 0, _mul46i = 0;
                c_mul(j, 0, M_PI, 0, &_mul46r, &_mul46i);
                double _c47r = 0, _c47i = 0;
                _c47r = 7.0; _c47i = 0;
                double _div48r = 0, _div48i = 0;
                c_div(_mul46r, _mul46i, _c47r, _c47i, &_div48r, &_div48i);
                double _sin49r = 0, _sin49i = 0;
                c_sin(_div48r, _div48i, &_sin49r, &_sin49i);
                double _add50r = 0, _add50i = 0;
                _add50r = _div45r + _sin49r; _add50i = _div45i + _sin49i;
                angle = _add50r;
            } else {
                double _c51r = 0, _c51i = 0;
                _c51r = 4.0; _c51i = 0;
                double _mod52r = 0, _mod52i = 0;
                _mod52r = fmod(j, _c51r); _mod52i = 0;
                double _c53r = 0, _c53i = 0;
                _c53r = 3.0; _c53i = 0;
                if (_mod52r == _c53r) {
                    double _re54r = 0, _re54i = 0;
                    _re54r = x1r; _re54i = 0;
                    double _mul55r = 0, _mul55i = 0;
                    c_mul(_re54r, _re54i, j, 0, &_mul55r, &_mul55i);
                    double _im56r = 0, _im56i = 0;
                    _im56r = x2i; _im56i = 0;
                    double _sub57r = 0, _sub57i = 0;
                    _sub57r = _mul55r - _im56r; _sub57i = _mul55i - _im56i;
                    double _add58r = 0, _add58i = 0;
                    _add58r = x1r + x2r; _add58i = x1i + x2i;
                    double _abs59r = 0, _abs59i = 0;
                    _abs59r = c_abs(_add58r, _add58i); _abs59i = 0;
                    double _c60r = 0, _c60i = 0;
                    _c60r = 1.0; _c60i = 0;
                    double _add61r = 0, _add61i = 0;
                    _add61r = _abs59r + _c60r; _add61i = _abs59i + _c60i;
                    double _log62r = 0, _log62i = 0;
                    c_log(_add61r, _add61i, &_log62r, &_log62i);
                    double _add63r = 0, _add63i = 0;
                    _add63r = _sub57r + _log62r; _add63i = _sub57i + _log62i;
                    mag = _add63r;
                    double _mul64r = 0, _mul64i = 0;
                    c_mul(x1r, x1i, x2r, x2i, &_mul64r, &_mul64i);
                    double _ang65r = 0, _ang65i = 0;
                    _ang65r = c_arg(_mul64r, _mul64i); _ang65i = 0;
                    double _mul66r = 0, _mul66i = 0;
                    c_mul(j, 0, M_PI, 0, &_mul66r, &_mul66i);
                    double _c67r = 0, _c67i = 0;
                    _c67r = 2.0; _c67i = 0;
                    double _div68r = 0, _div68i = 0;
                    c_div(_mul66r, _mul66i, _c67r, _c67i, &_div68r, &_div68i);
                    double _cos69r = 0, _cos69i = 0;
                    c_cos(_div68r, _div68i, &_cos69r, &_cos69i);
                    double _add70r = 0, _add70i = 0;
                    _add70r = _ang65r + _cos69r; _add70i = _ang65i + _cos69i;
                    angle = _add70r;
                } else {
                    double _sub71r = 0, _sub71i = 0;
                    _sub71r = x1r - x2r; _sub71i = x1i - x2i;
                    double _re72r = 0, _re72i = 0;
                    _re72r = _sub71r; _re72i = 0;
                    double _abs73r = 0, _abs73i = 0;
                    _abs73r = c_abs(_re72r, _re72i); _abs73i = 0;
                    double _c74r = 0, _c74i = 0;
                    _c74r = 1.5; _c74i = 0;
                    double _pow75r = 0, _pow75i = 0;
                    c_powr(j, 0, 1.5, &_pow75r, &_pow75i);
                    double _mul76r = 0, _mul76i = 0;
                    c_mul(_abs73r, _abs73i, _pow75r, _pow75i, &_mul76r, &_mul76i);
                    double _mul77r = 0, _mul77i = 0;
                    c_mul(j, 0, M_PI, 0, &_mul77r, &_mul77i);
                    double _c78r = 0, _c78i = 0;
                    _c78r = 3.0; _c78i = 0;
                    double _div79r = 0, _div79i = 0;
                    c_div(_mul77r, _mul77i, _c78r, _c78i, &_div79r, &_div79i);
                    double _sin80r = 0, _sin80i = 0;
                    c_sin(_div79r, _div79i, &_sin80r, &_sin80i);
                    double _add81r = 0, _add81i = 0;
                    _add81r = _mul76r + _sin80r; _add81i = _mul76i + _sin80i;
                    mag = _add81r;
                    double _sub82r = 0, _sub82i = 0;
                    _sub82r = x1r - x2r; _sub82i = x1i - x2i;
                    double _ang83r = 0, _ang83i = 0;
                    _ang83r = c_arg(_sub82r, _sub82i); _ang83i = 0;
                    double _mul84r = 0, _mul84i = 0;
                    c_mul(j, 0, M_PI, 0, &_mul84r, &_mul84i);
                    double _c85r = 0, _c85i = 0;
                    _c85r = 4.0; _c85i = 0;
                    double _div86r = 0, _div86i = 0;
                    c_div(_mul84r, _mul84i, _c85r, _c85i, &_div86r, &_div86i);
                    double _sin87r = 0, _sin87i = 0;
                    c_sin(_div86r, _div86i, &_sin87r, &_sin87i);
                    double _add88r = 0, _add88i = 0;
                    _add88r = _ang83r + _sin87r; _add88i = _ang83i + _sin87i;
                    angle = _add88r;
                }
            }
        }
        double _cos89r = 0, _cos89i = 0;
        c_cos(angle, 0, &_cos89r, &_cos89i);
        double _c90r = 0, _c90i = 0;
        _c90r = 0.0; _c90i = 1.0;
        double _sin91r = 0, _sin91i = 0;
        c_sin(angle, 0, &_sin91r, &_sin91i);
        double _mul92r = 0, _mul92i = 0;
        c_mul(_c90r, _c90i, _sin91r, _sin91i, &_mul92r, &_mul92i);
        double _add93r = 0, _add93i = 0;
        _add93r = _cos89r + _mul92r; _add93i = _cos89i + _mul92i;
        double _mul94r = 0, _mul94i = 0;
        c_mul(mag, 0, _add93r, _add93i, &_mul94r, &_mul94i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul94r; cIm[_idx] = _mul94i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_291_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
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
        c_mul(j, 0, M_PI, 0, &_mul13r, &_mul13i);
        double _c14r = 0, _c14i = 0;
        _c14r = 7.0; _c14i = 0;
        double _div15r = 0, _div15i = 0;
        c_div(_mul13r, _mul13i, _c14r, _c14i, &_div15r, &_div15i);
        double _sin16r = 0, _sin16i = 0;
        c_sin(_div15r, _div15i, &_sin16r, &_sin16i);
        double _add17r = 0, _add17i = 0;
        _add17r = _c12r + _sin16r; _add17i = _c12i + _sin16i;
        double _mul18r = 0, _mul18i = 0;
        c_mul(_log11r, _log11i, _add17r, _add17i, &_mul18r, &_mul18i);
        double mag_part1_r = _mul18r, mag_part1_i = _mul18i;
        double _c19r = 0, _c19i = 0;
        _c19r = 0.5; _c19i = 0;
        double _pow20r = 0, _pow20i = 0;
        c_powr(0, 0, 0.5, &_pow20r, &_pow20i);
        double _c21r = 0, _c21i = 0;
        _c21r = 1.0; _c21i = 0;
        double _sub22r = 0, _sub22i = 0;
        _sub22r = x1r - x2r; _sub22i = x1i - x2i;
        double _re23r = 0, _re23i = 0;
        _re23r = _sub22r; _re23i = 0;
        double _abs24r = 0, _abs24i = 0;
        _abs24r = c_abs(_re23r, _re23i); _abs24i = 0;
        double _c25r = 0, _c25i = 0;
        _c25r = 1.0; _c25i = 0;
        double _add26r = 0, _add26i = 0;
        _add26r = j + _c25r; _add26i = 0 + _c25i;
        double _div27r = 0, _div27i = 0;
        c_div(_abs24r, _abs24i, _add26r, _add26i, &_div27r, &_div27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _c21r + _div27r; _add28i = _c21i + _div27i;
        double _div29r = 0, _div29i = 0;
        c_div(_pow20r, _pow20i, _add28r, _add28i, &_div29r, &_div29i);
        double mag_part2_r = _div29r, mag_part2_i = _div29i;
        double _mul30r = 0, _mul30i = 0;
        c_mul(mag_part1_r, mag_part1_i, mag_part2_r, mag_part2_i, &_mul30r, &_mul30i);
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
        c_mul(_mul30r, _mul30i, _add36r, _add36i, &_mul37r, &_mul37i);
        double magnitude_r = _mul37r, magnitude_i = _mul37i;
        double _ang38r = 0, _ang38i = 0;
        _ang38r = c_arg(x1r, x1i); _ang38i = 0;
        double _c39r = 0, _c39i = 0;
        _c39r = 3.0; _c39i = 0;
        double _div40r = 0, _div40i = 0;
        c_div(j, 0, _c39r, _c39i, &_div40r, &_div40i);
        double _sin41r = 0, _sin41i = 0;
        c_sin(_div40r, _div40i, &_sin41r, &_sin41i);
        double _mul42r = 0, _mul42i = 0;
        c_mul(_ang38r, _ang38i, _sin41r, _sin41i, &_mul42r, &_mul42i);
        double _ang43r = 0, _ang43i = 0;
        _ang43r = c_arg(x2r, x2i); _ang43i = 0;
        double _c44r = 0, _c44i = 0;
        _c44r = 4.0; _c44i = 0;
        double _div45r = 0, _div45i = 0;
        c_div(j, 0, _c44r, _c44i, &_div45r, &_div45i);
        double _cos46r = 0, _cos46i = 0;
        c_cos(_div45r, _div45i, &_cos46r, &_cos46i);
        double _mul47r = 0, _mul47i = 0;
        c_mul(_ang43r, _ang43i, _cos46r, _cos46i, &_mul47r, &_mul47i);
        double _add48r = 0, _add48i = 0;
        _add48r = _mul42r + _mul47r; _add48i = _mul42i + _mul47i;
        double angle_part1_r = _add48r, angle_part1_i = _add48i;
        double _mul49r = 0, _mul49i = 0;
        c_mul(0, 0, M_PI, 0, &_mul49r, &_mul49i);
        double _c50r = 0, _c50i = 0;
        _c50r = 6.0; _c50i = 0;
        double _div51r = 0, _div51i = 0;
        c_div(_mul49r, _mul49i, _c50r, _c50i, &_div51r, &_div51i);
        double _sin52r = 0, _sin52i = 0;
        c_sin(_div51r, _div51i, &_sin52r, &_sin52i);
        double _mul53r = 0, _mul53i = 0;
        c_mul(0, 0, M_PI, 0, &_mul53r, &_mul53i);
        double _c54r = 0, _c54i = 0;
        _c54r = 8.0; _c54i = 0;
        double _div55r = 0, _div55i = 0;
        c_div(_mul53r, _mul53i, _c54r, _c54i, &_div55r, &_div55i);
        double _cos56r = 0, _cos56i = 0;
        c_cos(_div55r, _div55i, &_cos56r, &_cos56i);
        double _sub57r = 0, _sub57i = 0;
        _sub57r = _sin52r - _cos56r; _sub57i = _sin52i - _cos56i;
        double angle_part2_r = _sub57r, angle_part2_i = _sub57i;
        double _add58r = 0, _add58i = 0;
        _add58r = angle_part1_r + angle_part2_r; _add58i = angle_part1_i + angle_part2_i;
        double angle_r = _add58r, angle_i = _add58i;
        double _re59r = 0, _re59i = 0;
        _re59r = x1r; _re59i = 0;
        double _cos60r = 0, _cos60i = 0;
        c_cos(j, 0, &_cos60r, &_cos60i);
        double _mul61r = 0, _mul61i = 0;
        c_mul(_re59r, _re59i, _cos60r, _cos60i, &_mul61r, &_mul61i);
        double _im62r = 0, _im62i = 0;
        _im62r = x2i; _im62i = 0;
        double _sin63r = 0, _sin63i = 0;
        c_sin(j, 0, &_sin63r, &_sin63i);
        double _mul64r = 0, _mul64i = 0;
        c_mul(_im62r, _im62i, _sin63r, _sin63i, &_mul64r, &_mul64i);
        double _sub65r = 0, _sub65i = 0;
        _sub65r = _mul61r - _mul64r; _sub65i = _mul61i - _mul64i;
        double real_component_r = _sub65r, real_component_i = _sub65i;
        double _re66r = 0, _re66i = 0;
        _re66r = x2r; _re66i = 0;
        double _sin67r = 0, _sin67i = 0;
        c_sin(j, 0, &_sin67r, &_sin67i);
        double _mul68r = 0, _mul68i = 0;
        c_mul(_re66r, _re66i, _sin67r, _sin67i, &_mul68r, &_mul68i);
        double _im69r = 0, _im69i = 0;
        _im69r = x1i; _im69i = 0;
        double _cos70r = 0, _cos70i = 0;
        c_cos(j, 0, &_cos70r, &_cos70i);
        double _mul71r = 0, _mul71i = 0;
        c_mul(_im69r, _im69i, _cos70r, _cos70i, &_mul71r, &_mul71i);
        double _add72r = 0, _add72i = 0;
        _add72r = _mul68r + _mul71r; _add72i = _mul68i + _mul71i;
        double imag_component_r = _add72r, imag_component_i = _add72i;
        double _sin73r = 0, _sin73i = 0;
        c_sin(real_component_r, real_component_i, &_sin73r, &_sin73i);
        double _cos74r = 0, _cos74i = 0;
        c_cos(imag_component_r, imag_component_i, &_cos74r, &_cos74i);
        double _add75r = 0, _add75i = 0;
        _add75r = _sin73r + _cos74r; _add75i = _sin73i + _cos74i;
        double perturbation_r = _add75r, perturbation_i = _add75i;
        double _c76r = 0, _c76i = 0;
        _c76r = 0.0; _c76i = 1.0;
        double _mul77r = 0, _mul77i = 0;
        c_mul(_c76r, _c76i, angle_r, angle_i, &_mul77r, &_mul77i);
        double _exp78r = 0, _exp78i = 0;
        c_exp2(_mul77r, _mul77i, &_exp78r, &_exp78i);
        double _mul79r = 0, _mul79i = 0;
        c_mul(magnitude_r, magnitude_i, _exp78r, _exp78i, &_mul79r, &_mul79i);
        double _mul80r = 0, _mul80i = 0;
        c_mul(_mul79r, _mul79i, perturbation_r, perturbation_i, &_mul80r, &_mul80i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul80r; cIm[_idx] = _mul80i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_292_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
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
        double _c5r = 0, _c5i = 0;
        _c5r = 3.0; _c5i = 0;
        double _mul6r = 0, _mul6i = 0;
        c_mul(_c5r, _c5i, j, 0, &_mul6r, &_mul6i);
        double _add7r = 0, _add7i = 0;
        _add7r = _pow4r + _mul6r; _add7i = _pow4i + _mul6i;
        double _c8r = 0, _c8i = 0;
        _c8r = 1.0; _c8i = 0;
        double _add9r = 0, _add9i = 0;
        _add9r = _add7r + _c8r; _add9i = _add7i + _c8i;
        double _mod10r = 0, _mod10i = 0;
        _mod10r = fmod(_add9r, n); _mod10i = 0;
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = _mod10r + _c11r; _add12i = _mod10i + _c11i;
        double k_r = _add12r, k_i = _add12i;
        double _re13r = 0, _re13i = 0;
        _re13r = x1r; _re13i = 0;
        double _mul14r = 0, _mul14i = 0;
        c_mul(j, 0, _re13r, _re13i, &_mul14r, &_mul14i);
        double _sin15r = 0, _sin15i = 0;
        c_sin(_mul14r, _mul14i, &_sin15r, &_sin15i);
        double _im16r = 0, _im16i = 0;
        _im16r = x2i; _im16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(k_r, k_i, _im16r, _im16i, &_mul17r, &_mul17i);
        double _cos18r = 0, _cos18i = 0;
        c_cos(_mul17r, _mul17i, &_cos18r, &_cos18i);
        double _mul19r = 0, _mul19i = 0;
        c_mul(_sin15r, _sin15i, _cos18r, _cos18i, &_mul19r, &_mul19i);
        double r_r = _mul19r, r_i = _mul19i;
        double _ang20r = 0, _ang20i = 0;
        _ang20r = c_arg(x1r, x1i); _ang20i = 0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(_ang20r, _ang20i, j, 0, &_mul21r, &_mul21i);
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x2r, x2i); _ang22i = 0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(_ang22r, _ang22i, k_r, k_i, &_mul23r, &_mul23i);
        double _sub24r = 0, _sub24i = 0;
        _sub24r = _mul21r - _mul23r; _sub24i = _mul21i - _mul23i;
        double _c25r = 0, _c25i = 0;
        _c25r = 1.0; _c25i = 0;
        double _add26r = 0, _add26i = 0;
        _add26r = j + _c25r; _add26i = 0 + _c25i;
        double _log27r = 0, _log27i = 0;
        c_log(_add26r, _add26i, &_log27r, &_log27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _sub24r + _log27r; _add28i = _sub24i + _log27i;
        double angle_r = _add28r, angle_i = _add28i;
        double _abs29r = 0, _abs29i = 0;
        _abs29r = c_abs(x1r, x1i); _abs29i = 0;
        double _c30r = 0, _c30i = 0;
        _c30r = 0.5; _c30i = 0;
        double _pow31r = 0, _pow31i = 0;
        c_powr(_abs29r, _abs29i, 0.5, &_pow31r, &_pow31i);
        double _abs32r = 0, _abs32i = 0;
        _abs32r = c_abs(x2r, x2i); _abs32i = 0;
        double _c33r = 0, _c33i = 0;
        _c33r = 0.3; _c33i = 0;
        double _pow34r = 0, _pow34i = 0;
        c_powr(_abs32r, _abs32i, 0.3, &_pow34r, &_pow34i);
        double _mul35r = 0, _mul35i = 0;
        c_mul(_pow31r, _pow31i, _pow34r, _pow34i, &_mul35r, &_mul35i);
        double _abs36r = 0, _abs36i = 0;
        _abs36r = c_abs(r_r, r_i); _abs36i = 0;
        double _mul37r = 0, _mul37i = 0;
        c_mul(_mul35r, _mul35i, _abs36r, _abs36i, &_mul37r, &_mul37i);
        double _add38r = 0, _add38i = 0;
        _add38r = _mul37r + j; _add38i = _mul37i + 0;
        double magnitude_r = _add38r, magnitude_i = _add38i;
        double _cos39r = 0, _cos39i = 0;
        c_cos(angle_r, angle_i, &_cos39r, &_cos39i);
        double _c40r = 0, _c40i = 0;
        _c40r = 0.0; _c40i = 1.0;
        double _sin41r = 0, _sin41i = 0;
        c_sin(angle_r, angle_i, &_sin41r, &_sin41i);
        double _mul42r = 0, _mul42i = 0;
        c_mul(_c40r, _c40i, _sin41r, _sin41i, &_mul42r, &_mul42i);
        double _add43r = 0, _add43i = 0;
        _add43r = _cos39r + _mul42r; _add43i = _cos39i + _mul42i;
        double _mul44r = 0, _mul44i = 0;
        c_mul(magnitude_r, magnitude_i, _add43r, _add43i, &_mul44r, &_mul44i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul44r; cIm[_idx] = _mul44i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_293_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _c3r = 0, _c3i = 0;
        _c3r = 2.0; _c3i = 0;
        double _mul4r = 0, _mul4i = 0;
        c_mul(j, 0, _c3r, _c3i, &_mul4r, &_mul4i);
        double _c5r = 0, _c5i = 0;
        _c5r = 5.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = _mul4r + _c5r; _add6i = _mul4i + _c5i;
        double _c7r = 0, _c7i = 0;
        _c7r = 12.0; _c7i = 0;
        double _mod8r = 0, _mod8i = 0;
        _mod8r = fmod(_add6r, _c7r); _mod8i = 0;
        double k_r = _mod8r, k_i = _mod8i;
        double _c9r = 0, _c9i = 0;
        _c9r = 6.0; _c9i = 0;
        double _fdiv10r = 0, _fdiv10i = 0;
        c_div(j, 0, _c9r, _c9i, &_fdiv10r, &_fdiv10i);
        _fdiv10r = floor(_fdiv10r); _fdiv10i = 0;
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = _fdiv10r + _c11r; _add12i = _fdiv10i + _c11i;
        double r_r = _add12r, r_i = _add12i;
        double _re13r = 0, _re13i = 0;
        _re13r = x1r; _re13i = 0;
        double _sin14r = 0, _sin14i = 0;
        c_sin(j, 0, &_sin14r, &_sin14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_re13r, _re13i, _sin14r, _sin14i, &_mul15r, &_mul15i);
        double _re16r = 0, _re16i = 0;
        _re16r = x2r; _re16i = 0;
        double _cos17r = 0, _cos17i = 0;
        c_cos(k_r, k_i, &_cos17r, &_cos17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_re16r, _re16i, _cos17r, _cos17i, &_mul18r, &_mul18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _mul15r + _mul18r; _add19i = _mul15i + _mul18i;
        double term_re_r = _add19r, term_re_i = _add19i;
        double _im20r = 0, _im20i = 0;
        _im20r = x1i; _im20i = 0;
        double _c21r = 0, _c21i = 0;
        _c21r = 4.0; _c21i = 0;
        double _div22r = 0, _div22i = 0;
        c_div(j, 0, _c21r, _c21i, &_div22r, &_div22i);
        double _cos23r = 0, _cos23i = 0;
        c_cos(_div22r, _div22i, &_cos23r, &_cos23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_im20r, _im20i, _cos23r, _cos23i, &_mul24r, &_mul24i);
        double _im25r = 0, _im25i = 0;
        _im25r = x2i; _im25i = 0;
        double _c26r = 0, _c26i = 0;
        _c26r = 3.0; _c26i = 0;
        double _div27r = 0, _div27i = 0;
        c_div(k_r, k_i, _c26r, _c26i, &_div27r, &_div27i);
        double _sin28r = 0, _sin28i = 0;
        c_sin(_div27r, _div27i, &_sin28r, &_sin28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_im25r, _im25i, _sin28r, _sin28i, &_mul29r, &_mul29i);
        double _sub30r = 0, _sub30i = 0;
        _sub30r = _mul24r - _mul29r; _sub30i = _mul24i - _mul29i;
        double term_im_r = _sub30r, term_im_i = _sub30i;
        double _abs31r = 0, _abs31i = 0;
        _abs31r = c_abs(term_re_r, term_re_i); _abs31i = 0;
        double _abs32r = 0, _abs32i = 0;
        _abs32r = c_abs(term_im_r, term_im_i); _abs32i = 0;
        double _add33r = 0, _add33i = 0;
        _add33r = _abs31r + _abs32r; _add33i = _abs31i + _abs32i;
        double _c34r = 0, _c34i = 0;
        _c34r = 1.0; _c34i = 0;
        double _add35r = 0, _add35i = 0;
        _add35r = _c34r + j; _add35i = _c34i + 0;
        double _log36r = 0, _log36i = 0;
        c_log(_add35r, _add35i, &_log36r, &_log36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(_add33r, _add33i, _log36r, _log36i, &_mul37r, &_mul37i);
        double _c38r = 0, _c38i = 0;
        _c38r = 0.4; _c38i = 0;
        double _pow39r = 0, _pow39i = 0;
        c_powr(j, 0, 0.4, &_pow39r, &_pow39i);
        double _mul40r = 0, _mul40i = 0;
        c_mul(_mul37r, _mul37i, _pow39r, _pow39i, &_mul40r, &_mul40i);
        double magnitude_r = _mul40r, magnitude_i = _mul40i;
        double _ang41r = 0, _ang41i = 0;
        _ang41r = c_arg(x1r, x1i); _ang41i = 0;
        double _c42r = 0, _c42i = 0;
        _c42r = 2.0; _c42i = 0;
        double _div43r = 0, _div43i = 0;
        c_div(j, 0, _c42r, _c42i, &_div43r, &_div43i);
        double _sin44r = 0, _sin44i = 0;
        c_sin(_div43r, _div43i, &_sin44r, &_sin44i);
        double _mul45r = 0, _mul45i = 0;
        c_mul(_ang41r, _ang41i, _sin44r, _sin44i, &_mul45r, &_mul45i);
        double _ang46r = 0, _ang46i = 0;
        _ang46r = c_arg(x2r, x2i); _ang46i = 0;
        double _c47r = 0, _c47i = 0;
        _c47r = 4.0; _c47i = 0;
        double _div48r = 0, _div48i = 0;
        c_div(k_r, k_i, _c47r, _c47i, &_div48r, &_div48i);
        double _cos49r = 0, _cos49i = 0;
        c_cos(_div48r, _div48i, &_cos49r, &_cos49i);
        double _mul50r = 0, _mul50i = 0;
        c_mul(_ang46r, _ang46i, _cos49r, _cos49i, &_mul50r, &_mul50i);
        double _add51r = 0, _add51i = 0;
        _add51r = _mul45r + _mul50r; _add51i = _mul45i + _mul50i;
        double _c52r = 0, _c52i = 0;
        _c52r = 2.0; _c52i = 0;
        double _add53r = 0, _add53i = 0;
        _add53r = j + _c52r; _add53i = 0 + _c52i;
        double _log54r = 0, _log54i = 0;
        c_log(_add53r, _add53i, &_log54r, &_log54i);
        double _add55r = 0, _add55i = 0;
        _add55r = _add51r + _log54r; _add55i = _add51i + _log54i;
        double angle_r = _add55r, angle_i = _add55i;
        double _c56r = 0, _c56i = 0;
        _c56r = 0.0; _c56i = 1.0;
        double _mul57r = 0, _mul57i = 0;
        c_mul(_c56r, _c56i, angle_r, angle_i, &_mul57r, &_mul57i);
        double _exp58r = 0, _exp58i = 0;
        c_exp2(_mul57r, _mul57i, &_exp58r, &_exp58i);
        double _mul59r = 0, _mul59i = 0;
        c_mul(magnitude_r, magnitude_i, _exp58r, _exp58i, &_mul59r, &_mul59i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul59r; cIm[_idx] = _mul59i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_294_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
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
        double _c7r = 0, _c7i = 0;
        _c7r = 7.0; _c7i = 0;
        double _mod8r = 0, _mod8i = 0;
        _mod8r = fmod(j, _c7r); _mod8i = 0;
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = _mod8r + _c9r; _add10i = _mod8i + _c9i;
        double k_r = _add10r, k_i = _add10i;
        double _arr11r = 0, _arr11i = 0;
        { int _idx = (j - 1); _arr11r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr11i = 0; }
        double _cos12r = 0, _cos12i = 0;
        c_cos(j, 0, &_cos12r, &_cos12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_arr11r, _arr11i, _cos12r, _cos12i, &_mul13r, &_mul13i);
        double _arr14r = 0, _arr14i = 0;
        { int _idx = (j - 1); _arr14r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr14i = 0; }
        double _sin15r = 0, _sin15i = 0;
        c_sin(j, 0, &_sin15r, &_sin15i);
        double _mul16r = 0, _mul16i = 0;
        c_mul(_arr14r, _arr14i, _sin15r, _sin15i, &_mul16r, &_mul16i);
        double _sub17r = 0, _sub17i = 0;
        _sub17r = _mul13r - _mul16r; _sub17i = _mul13i - _mul16i;
        double r_r = _sub17r, r_i = _sub17i;
        double _arr18r = 0, _arr18i = 0;
        { int _idx = (j - 1); _arr18r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr18i = 0; }
        double _sin19r = 0, _sin19i = 0;
        c_sin(j, 0, &_sin19r, &_sin19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_arr18r, _arr18i, _sin19r, _sin19i, &_mul20r, &_mul20i);
        double _arr21r = 0, _arr21i = 0;
        { int _idx = (j - 1); _arr21r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr21i = 0; }
        double _cos22r = 0, _cos22i = 0;
        c_cos(j, 0, &_cos22r, &_cos22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_arr21r, _arr21i, _cos22r, _cos22i, &_mul23r, &_mul23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _mul20r + _mul23r; _add24i = _mul20i + _mul23i;
        double i_part_r = _add24r, i_part_i = _add24i;
        double _c25r = 0, _c25i = 0;
        _c25r = 1.0; _c25i = 0;
        double _add26r = 0, _add26i = 0;
        _add26r = r_r + _c25r; _add26i = r_i + _c25i;
        double _abs27r = 0, _abs27i = 0;
        _abs27r = c_abs(_add26r, _add26i); _abs27i = 0;
        double _c28r = 0, _c28i = 0;
        _c28r = 1.0; _c28i = 0;
        double _add29r = 0, _add29i = 0;
        _add29r = i_part_r + _c28r; _add29i = i_part_i + _c28i;
        double _abs30r = 0, _abs30i = 0;
        _abs30r = c_abs(_add29r, _add29i); _abs30i = 0;
        double _add31r = 0, _add31i = 0;
        _add31r = _abs27r + _abs30r; _add31i = _abs27i + _abs30i;
        double _log32r = 0, _log32i = 0;
        c_log(_add31r, _add31i, &_log32r, &_log32i);
        double _c33r = 0, _c33i = 0;
        _c33r = 1.0; _c33i = 0;
        double _mul34r = 0, _mul34i = 0;
        c_mul(j, 0, M_PI, 0, &_mul34r, &_mul34i);
        double _div35r = 0, _div35i = 0;
        c_div(_mul34r, _mul34i, k_r, k_i, &_div35r, &_div35i);
        double _sin36r = 0, _sin36i = 0;
        c_sin(_div35r, _div35i, &_sin36r, &_sin36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _c33r + _sin36r; _add37i = _c33i + _sin36i;
        double _mul38r = 0, _mul38i = 0;
        c_mul(_log32r, _log32i, _add37r, _add37i, &_mul38r, &_mul38i);
        double _c39r = 0, _c39i = 0;
        _c39r = 1.0; _c39i = 0;
        double _mul40r = 0, _mul40i = 0;
        c_mul(j, 0, M_PI, 0, &_mul40r, &_mul40i);
        double _c41r = 0, _c41i = 0;
        _c41r = 1.0; _c41i = 0;
        double _add42r = 0, _add42i = 0;
        _add42r = k_r + _c41r; _add42i = k_i + _c41i;
        double _div43r = 0, _div43i = 0;
        c_div(_mul40r, _mul40i, _add42r, _add42i, &_div43r, &_div43i);
        double _cos44r = 0, _cos44i = 0;
        c_cos(_div43r, _div43i, &_cos44r, &_cos44i);
        double _add45r = 0, _add45i = 0;
        _add45r = _c39r + _cos44r; _add45i = _c39i + _cos44i;
        double _mul46r = 0, _mul46i = 0;
        c_mul(_mul38r, _mul38i, _add45r, _add45i, &_mul46r, &_mul46i);
        double mag_r = _mul46r, mag_i = _mul46i;
        double _ang47r = 0, _ang47i = 0;
        _ang47r = c_arg(x1r, x1i); _ang47i = 0;
        double _ang48r = 0, _ang48i = 0;
        _ang48r = c_arg(x2r, x2i); _ang48i = 0;
        double _add49r = 0, _add49i = 0;
        _add49r = _ang47r + _ang48r; _add49i = _ang47i + _ang48i;
        double _mul50r = 0, _mul50i = 0;
        c_mul(j, 0, M_PI, 0, &_mul50r, &_mul50i);
        double _div51r = 0, _div51i = 0;
        c_div(_mul50r, _mul50i, k_r, k_i, &_div51r, &_div51i);
        double _sin52r = 0, _sin52i = 0;
        c_sin(_div51r, _div51i, &_sin52r, &_sin52i);
        double _add53r = 0, _add53i = 0;
        _add53r = _add49r + _sin52r; _add53i = _add49i + _sin52i;
        double _mul54r = 0, _mul54i = 0;
        c_mul(j, 0, M_PI, 0, &_mul54r, &_mul54i);
        double _c55r = 0, _c55i = 0;
        _c55r = 2.0; _c55i = 0;
        double _add56r = 0, _add56i = 0;
        _add56r = k_r + _c55r; _add56i = k_i + _c55i;
        double _div57r = 0, _div57i = 0;
        c_div(_mul54r, _mul54i, _add56r, _add56i, &_div57r, &_div57i);
        double _cos58r = 0, _cos58i = 0;
        c_cos(_div57r, _div57i, &_cos58r, &_cos58i);
        double _add59r = 0, _add59i = 0;
        _add59r = _add53r + _cos58r; _add59i = _add53i + _cos58i;
        double angle_r = _add59r, angle_i = _add59i;
        double _c60r = 0, _c60i = 0;
        _c60r = 0.0; _c60i = 1.0;
        double _mul61r = 0, _mul61i = 0;
        c_mul(_c60r, _c60i, angle_r, angle_i, &_mul61r, &_mul61i);
        double _exp62r = 0, _exp62i = 0;
        c_exp2(_mul61r, _mul61i, &_exp62r, &_exp62i);
        double _mul63r = 0, _mul63i = 0;
        c_mul(mag_r, mag_i, _exp62r, _exp62i, &_mul63r, &_mul63i);
        double _mul64r = 0, _mul64i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul64r, &_mul64i);
        double _conj65r = 0, _conj65i = 0;
        _conj65r = _mul64r; _conj65i = -(_mul64i);
        double _c66r = 0, _c66i = 0;
        _c66r = 2.0; _c66i = 0;
        double _add67r = 0, _add67i = 0;
        _add67r = j + _c66r; _add67i = 0 + _c66i;
        double _div68r = 0, _div68i = 0;
        c_div(_conj65r, _conj65i, _add67r, _add67i, &_div68r, &_div68i);
        double _add69r = 0, _add69i = 0;
        _add69r = _mul63r + _div68r; _add69i = _mul63i + _div68i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add69r; cIm[_idx] = _add69i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_295_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
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
        double _arr8r = 0, _arr8i = 0;
        { int _idx = (j - 1); _arr8r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr8i = 0; }
        double _add9r = 0, _add9i = 0;
        _add9r = _arr7r + _arr8r; _add9i = _arr7i + _arr8i;
        double r_r = _add9r, r_i = _add9i;
        double _c10r = 0, _c10i = 0;
        _c10r = 0.0; _c10i = 1.0;
        double _add11r = 0, _add11i = 0;
        _add11r = r_r + _c10r; _add11i = r_i + _c10i;
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(_add11r, _add11i); _abs12i = 0;
        double _c13r = 0, _c13i = 0;
        _c13r = 1.0; _c13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = _abs12r + _c13r; _add14i = _abs12i + _c13i;
        double _log15r = 0, _log15i = 0;
        c_log(_add14r, _add14i, &_log15r, &_log15i);
        double _sin16r = 0, _sin16i = 0;
        c_sin(j, 0, &_sin16r, &_sin16i);
        double _c17r = 0, _c17i = 0;
        _c17r = 1.0; _c17i = 0;
        double _add18r = 0, _add18i = 0;
        _add18r = _sin16r + _c17r; _add18i = _sin16i + _c17i;
        double _pow19r = 0, _pow19i = 0;
        c_powc(j, 0, _add18r, _add18i, &_pow19r, &_pow19i);
        double _mul20r = 0, _mul20i = 0;
        c_mul(_log15r, _log15i, _pow19r, _pow19i, &_mul20r, &_mul20i);
        double magnitude_r = _mul20r, magnitude_i = _mul20i;
        double _c21r = 0, _c21i = 0;
        _c21r = 0.0; _c21i = 1.0;
        double _add22r = 0, _add22i = 0;
        _add22r = r_r + _c21r; _add22i = r_i + _c21i;
        double _ang23r = 0, _ang23i = 0;
        _ang23r = c_arg(_add22r, _add22i); _ang23i = 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(j, 0, M_PI, 0, &_mul24r, &_mul24i);
        double _c25r = 0, _c25i = 0;
        _c25r = 4.0; _c25i = 0;
        double _div26r = 0, _div26i = 0;
        c_div(_mul24r, _mul24i, _c25r, _c25i, &_div26r, &_div26i);
        double _sin27r = 0, _sin27i = 0;
        c_sin(_div26r, _div26i, &_sin27r, &_sin27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(j, 0, M_PI, 0, &_mul28r, &_mul28i);
        double _c29r = 0, _c29i = 0;
        _c29r = 3.0; _c29i = 0;
        double _div30r = 0, _div30i = 0;
        c_div(_mul28r, _mul28i, _c29r, _c29i, &_div30r, &_div30i);
        double _cos31r = 0, _cos31i = 0;
        c_cos(_div30r, _div30i, &_cos31r, &_cos31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(_sin27r, _sin27i, _cos31r, _cos31i, &_mul32r, &_mul32i);
        double _add33r = 0, _add33i = 0;
        _add33r = _ang23r + _mul32r; _add33i = _ang23i + _mul32i;
        double angle_r = _add33r, angle_i = _add33i;
        double _c34r = 0, _c34i = 0;
        _c34r = 0.0; _c34i = 1.0;
        double _mul35r = 0, _mul35i = 0;
        c_mul(_c34r, _c34i, angle_r, angle_i, &_mul35r, &_mul35i);
        double _exp36r = 0, _exp36i = 0;
        c_exp2(_mul35r, _mul35i, &_exp36r, &_exp36i);
        double _mul37r = 0, _mul37i = 0;
        c_mul(magnitude_r, magnitude_i, _exp36r, _exp36i, &_mul37r, &_mul37i);
        double _c38r = 0, _c38i = 0;
        _c38r = 0.0; _c38i = 1.0;
        double _c39r = 0, _c39i = 0;
        _c39r = 2.0; _c39i = 0;
        double _div40r = 0, _div40i = 0;
        c_div(angle_r, angle_i, _c39r, _c39i, &_div40r, &_div40i);
        double _mul41r = 0, _mul41i = 0;
        c_mul(_c38r, _c38i, _div40r, _div40i, &_mul41r, &_mul41i);
        double _exp42r = 0, _exp42i = 0;
        c_exp2(_mul41r, _mul41i, &_exp42r, &_exp42i);
        double _mul43r = 0, _mul43i = 0;
        c_mul(magnitude_r, magnitude_i, _exp42r, _exp42i, &_mul43r, &_mul43i);
        double _conj44r = 0, _conj44i = 0;
        _conj44r = _mul43r; _conj44i = -(_mul43i);
        double _mul45r = 0, _mul45i = 0;
        c_mul(j, 0, M_PI, 0, &_mul45r, &_mul45i);
        double _c46r = 0, _c46i = 0;
        _c46r = 5.0; _c46i = 0;
        double _div47r = 0, _div47i = 0;
        c_div(_mul45r, _mul45i, _c46r, _c46i, &_div47r, &_div47i);
        double _cos48r = 0, _cos48i = 0;
        c_cos(_div47r, _div47i, &_cos48r, &_cos48i);
        double _mul49r = 0, _mul49i = 0;
        c_mul(_conj44r, _conj44i, _cos48r, _cos48i, &_mul49r, &_mul49i);
        double _add50r = 0, _add50i = 0;
        _add50r = _mul37r + _mul49r; _add50i = _mul37i + _mul49i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add50r; cIm[_idx] = _add50i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_296_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
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
    for (int k = 1; k < 36; k++) {
        double _c5r = 0, _c5i = 0;
        _c5r = 7.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = k + _c5r; _add6i = 0 + _c5i;
        double _c7r = 0, _c7i = 0;
        _c7r = 12.0; _c7i = 0;
        double _mod8r = 0, _mod8i = 0;
        _mod8r = fmod(_add6r, _c7r); _mod8i = 0;
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = _mod8r + _c9r; _add10i = _mod8i + _c9i;
        double j_r = _add10r, j_i = _add10i;
        double _arr11r = 0, _arr11i = 0;
        { int _idx = (k - 1); _arr11r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr11i = 0; }
        double _arr12r = 0, _arr12i = 0;
        { int _idx = ((int)(j_r) - 1); _arr12r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr12i = 0; }
        double _mul13r = 0, _mul13i = 0;
        c_mul(_arr12r, _arr12i, M_PI, 0, &_mul13r, &_mul13i);
        double _c14r = 0, _c14i = 0;
        _c14r = 5.0; _c14i = 0;
        double _div15r = 0, _div15i = 0;
        c_div(_mul13r, _mul13i, _c14r, _c14i, &_div15r, &_div15i);
        double _cos16r = 0, _cos16i = 0;
        c_cos(_div15r, _div15i, &_cos16r, &_cos16i);
        double _mul17r = 0, _mul17i = 0;
        c_mul(_arr11r, _arr11i, _cos16r, _cos16i, &_mul17r, &_mul17i);
        double term1_r = _mul17r, term1_i = _mul17i;
        double _arr18r = 0, _arr18i = 0;
        { int _idx = (k - 1); _arr18r = (_idx >= 0 && _idx < 35) ? imc[_idx] : 0.0; _arr18i = 0; }
        double _arr19r = 0, _arr19i = 0;
        { int _idx = ((int)(j_r) - 1); _arr19r = (_idx >= 0 && _idx < 35) ? rec[_idx] : 0.0; _arr19i = 0; }
        double _mul20r = 0, _mul20i = 0;
        c_mul(_arr19r, _arr19i, M_PI, 0, &_mul20r, &_mul20i);
        double _c21r = 0, _c21i = 0;
        _c21r = 4.0; _c21i = 0;
        double _div22r = 0, _div22i = 0;
        c_div(_mul20r, _mul20i, _c21r, _c21i, &_div22r, &_div22i);
        double _sin23r = 0, _sin23i = 0;
        c_sin(_div22r, _div22i, &_sin23r, &_sin23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_arr18r, _arr18i, _sin23r, _sin23i, &_mul24r, &_mul24i);
        double term2_r = _mul24r, term2_i = _mul24i;
        double _conj25r = 0, _conj25i = 0;
        _conj25r = x1r; _conj25i = -(x1i);
        double _conj26r = 0, _conj26i = 0;
        _conj26r = x2r; _conj26i = -(x2i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_conj25r, _conj25i, _conj26r, _conj26i, &_mul27r, &_mul27i);
        double conj_part_r = _mul27r, conj_part_i = _mul27i;
        double _add28r = 0, _add28i = 0;
        _add28r = term1_r + term2_r; _add28i = term1_i + term2_i;
        double _ang29r = 0, _ang29i = 0;
        _ang29r = c_arg(conj_part_r, conj_part_i); _ang29i = 0;
        double _add30r = 0, _add30i = 0;
        _add30r = _add28r + _ang29r; _add30i = _add28i + _ang29i;
        double _ang31r = 0, _ang31i = 0;
        _ang31r = c_arg(_add30r, _add30i); _ang31i = 0;
        double angle_r = _ang31r, angle_i = _ang31i;
        double _add32r = 0, _add32i = 0;
        _add32r = term1_r + term2_r; _add32i = term1_i + term2_i;
        double _abs33r = 0, _abs33i = 0;
        _abs33r = c_abs(_add32r, _add32i); _abs33i = 0;
        double _c34r = 0, _c34i = 0;
        _c34r = 1.0; _c34i = 0;
        double _add35r = 0, _add35i = 0;
        _add35r = _abs33r + _c34r; _add35i = _abs33i + _c34i;
        double _log36r = 0, _log36i = 0;
        c_log(_add35r, _add35i, &_log36r, &_log36i);
        double _abs37r = 0, _abs37i = 0;
        _abs37r = c_abs(x1r, x1i); _abs37i = 0;
        double _c38r = 0, _c38i = 0;
        _c38r = 4.0; _c38i = 0;
        double _mod39r = 0, _mod39i = 0;
        _mod39r = fmod(k, _c38r); _mod39i = 0;
        double _c40r = 0, _c40i = 0;
        _c40r = 1.0; _c40i = 0;
        double _add41r = 0, _add41i = 0;
        _add41r = _mod39r + _c40r; _add41i = _mod39i + _c40i;
        double _pow42r = 0, _pow42i = 0;
        c_powc(_abs37r, _abs37i, _add41r, _add41i, &_pow42r, &_pow42i);
        double _mul43r = 0, _mul43i = 0;
        c_mul(_log36r, _log36i, _pow42r, _pow42i, &_mul43r, &_mul43i);
        double _abs44r = 0, _abs44i = 0;
        _abs44r = c_abs(x2r, x2i); _abs44i = 0;
        double _c45r = 0, _c45i = 0;
        _c45r = 3.0; _c45i = 0;
        double _mod46r = 0, _mod46i = 0;
        _mod46r = fmod(j_r, _c45r); _mod46i = 0;
        double _c47r = 0, _c47i = 0;
        _c47r = 1.0; _c47i = 0;
        double _add48r = 0, _add48i = 0;
        _add48r = _mod46r + _c47r; _add48i = _mod46i + _c47i;
        double _pow49r = 0, _pow49i = 0;
        c_powc(_abs44r, _abs44i, _add48r, _add48i, &_pow49r, &_pow49i);
        double _mul50r = 0, _mul50i = 0;
        c_mul(_mul43r, _mul43i, _pow49r, _pow49i, &_mul50r, &_mul50i);
        double magnitude_r = _mul50r, magnitude_i = _mul50i;
        double _cos51r = 0, _cos51i = 0;
        c_cos(angle_r, angle_i, &_cos51r, &_cos51i);
        double _c52r = 0, _c52i = 0;
        _c52r = 0.0; _c52i = 1.0;
        double _sin53r = 0, _sin53i = 0;
        c_sin(angle_r, angle_i, &_sin53r, &_sin53i);
        double _mul54r = 0, _mul54i = 0;
        c_mul(_c52r, _c52i, _sin53r, _sin53i, &_mul54r, &_mul54i);
        double _add55r = 0, _add55i = 0;
        _add55r = _cos51r + _mul54r; _add55i = _cos51i + _mul54i;
        double _mul56r = 0, _mul56i = 0;
        c_mul(magnitude_r, magnitude_i, _add55r, _add55i, &_mul56r, &_mul56i);
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul56r; cIm[_idx] = _mul56i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_297_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
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
        double _ang6r = 0, _ang6i = 0;
        _ang6r = c_arg(x2r, x2i); _ang6i = 0;
        double _mul7r = 0, _mul7i = 0;
        c_mul(j, 0, _ang6r, _ang6i, &_mul7r, &_mul7i);
        double _sin8r = 0, _sin8i = 0;
        c_sin(_mul7r, _mul7i, &_sin8r, &_sin8i);
        double _mul9r = 0, _mul9i = 0;
        c_mul(_log5r, _log5i, _sin8r, _sin8i, &_mul9r, &_mul9i);
        double _c10r = 0, _c10i = 0;
        _c10r = 2.0; _c10i = 0;
        double _pow11r = 0, _pow11i = 0;
        c_mul(j, 0, j, 0, &_pow11r, &_pow11i);
        double _re12r = 0, _re12i = 0;
        _re12r = x1r; _re12i = 0;
        double _mul13r = 0, _mul13i = 0;
        c_mul(_pow11r, _pow11i, _re12r, _re12i, &_mul13r, &_mul13i);
        double _cos14r = 0, _cos14i = 0;
        c_cos(_mul13r, _mul13i, &_cos14r, &_cos14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _mul9r + _cos14r; _add15i = _mul9i + _cos14i;
        double mag_part_r = _add15r, mag_part_i = _add15i;
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
        double _ang21r = 0, _ang21i = 0;
        _ang21r = c_arg(x2r, x2i); _ang21i = 0;
        double _sqrt22r = 0, _sqrt22i = 0;
        c_powr(j, 0, 0.5, &_sqrt22r, &_sqrt22i);
        double _mul23r = 0, _mul23i = 0;
        c_mul(_ang21r, _ang21i, _sqrt22r, _sqrt22i, &_mul23r, &_mul23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _mul20r + _mul23r; _add24i = _mul20i + _mul23i;
        double angle_part_r = _add24r, angle_part_i = _add24i;
        double _c25r = 0, _c25i = 0;
        _c25r = 0.0; _c25i = 1.0;
        double _mul26r = 0, _mul26i = 0;
        c_mul(_c25r, _c25i, angle_part_r, angle_part_i, &_mul26r, &_mul26i);
        double _exp27r = 0, _exp27i = 0;
        c_exp2(_mul26r, _mul26i, &_exp27r, &_exp27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(mag_part_r, mag_part_i, _exp27r, _exp27i, &_mul28r, &_mul28i);
        double _conj29r = 0, _conj29i = 0;
        _conj29r = x1r; _conj29i = -(x1i);
        double _pow30r = 0, _pow30i = 0;
        c_powr(_conj29r, _conj29i, j, &_pow30r, &_pow30i);
        double _c31r = 0, _c31i = 0;
        _c31r = 1.0; _c31i = 0;
        double _add32r = 0, _add32i = 0;
        _add32r = x2r + j; _add32i = x2i + 0;
        double _abs33r = 0, _abs33i = 0;
        _abs33r = c_abs(_add32r, _add32i); _abs33i = 0;
        double _add34r = 0, _add34i = 0;
        _add34r = _c31r + _abs33r; _add34i = _c31i + _abs33i;
        double _div35r = 0, _div35i = 0;
        c_div(_pow30r, _pow30i, _add34r, _add34i, &_div35r, &_div35i);
        double _add36r = 0, _add36i = 0;
        _add36r = _mul28r + _div35r; _add36i = _mul28i + _div35i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add36r; cIm[_idx] = _add36i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_298_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
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
        double _add21r = 0, _add21i = 0;
        _add21r = _mul16r + _cos20r; _add21i = _mul16i + _cos20i;
        double mag_part_r = _add21r, mag_part_i = _add21i;
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x1r, x1i); _ang22i = 0;
        double _c23r = 0, _c23i = 0;
        _c23r = 0.5; _c23i = 0;
        double _pow24r = 0, _pow24i = 0;
        c_powr(j, 0, 0.5, &_pow24r, &_pow24i);
        double _mul25r = 0, _mul25i = 0;
        c_mul(_ang22r, _ang22i, _pow24r, _pow24i, &_mul25r, &_mul25i);
        double _ang26r = 0, _ang26i = 0;
        _ang26r = c_arg(x2r, x2i); _ang26i = 0;
        double _c27r = 0, _c27i = 0;
        _c27r = 2.0; _c27i = 0;
        double _add28r = 0, _add28i = 0;
        _add28r = j + _c27r; _add28i = 0 + _c27i;
        double _div29r = 0, _div29i = 0;
        c_div(_ang26r, _ang26i, _add28r, _add28i, &_div29r, &_div29i);
        double _sub30r = 0, _sub30i = 0;
        _sub30r = _mul25r - _div29r; _sub30i = _mul25i - _div29i;
        double angle_part_r = _sub30r, angle_part_i = _sub30i;
        double _c31r = 0, _c31i = 0;
        _c31r = 3.0; _c31i = 0;
        double _mod32r = 0, _mod32i = 0;
        _mod32r = fmod(j, _c31r); _mod32i = 0;
        double _c33r = 0, _c33i = 0;
        _c33r = 0.0; _c33i = 0;
        double _abs34r = 0, _abs34i = 0;
        _abs34r = c_abs(x1r, x1i); _abs34i = 0;
        double _abs35r = 0, _abs35i = 0;
        _abs35r = c_abs(x2r, x2i); _abs35i = 0;
        double _mul36r = 0, _mul36i = 0;
        c_mul(_abs34r, _abs34i, _abs35r, _abs35i, &_mul36r, &_mul36i);
        double _add37r = 0, _add37i = 0;
        _add37r = x1r + x2r; _add37i = x1i + x2i;
        double _abs38r = 0, _abs38i = 0;
        _abs38r = c_abs(_add37r, _add37i); _abs38i = 0;
        double _c39r = 0, _c39i = 0;
        _c39r = 1.0; _c39i = 0;
        double _add40r = 0, _add40i = 0;
        _add40r = j + _c39r; _add40i = 0 + _c39i;
        double _div41r = 0, _div41i = 0;
        c_div(_abs38r, _abs38i, _add40r, _add40i, &_div41r, &_div41i);
        double _tern42r = 0, _tern42i = 0;
        if (_mod32r == _c33r) { _tern42r = _mul36r; _tern42i = _mul36i; }
        else { _tern42r = _div41r; _tern42i = _div41i; }
        double fluctuation_r = _tern42r, fluctuation_i = _tern42i;
        double _add43r = 0, _add43i = 0;
        _add43r = mag_part_r + fluctuation_r; _add43i = mag_part_i + fluctuation_i;
        double _c44r = 0, _c44i = 0;
        _c44r = 0.0; _c44i = 1.0;
        double _mul45r = 0, _mul45i = 0;
        c_mul(_c44r, _c44i, angle_part_r, angle_part_i, &_mul45r, &_mul45i);
        double _exp46r = 0, _exp46i = 0;
        c_exp2(_mul45r, _mul45i, &_exp46r, &_exp46i);
        double _mul47r = 0, _mul47i = 0;
        c_mul(_add43r, _add43i, _exp46r, _exp46i, &_mul47r, &_mul47i);
        double _mul48r = 0, _mul48i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul48r, &_mul48i);
        double _conj49r = 0, _conj49i = 0;
        _conj49r = _mul48r; _conj49i = -(_mul48i);
        double _pow50r = 0, _pow50i = 0;
        c_powr(_conj49r, _conj49i, j, &_pow50r, &_pow50i);
        double _add51r = 0, _add51i = 0;
        _add51r = _mul47r + _pow50r; _add51i = _mul47i + _pow50i;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _add51r; cIm[_idx] = _add51i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_299_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
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
        double _c7r = 0, _c7i = 0;
        _c7r = 1.0; _c7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _sub6r + _c7r; _add8i = _sub6i + _c7i;
        double _pow9r = 0, _pow9i = 0;
        c_powc(_abs5r, _abs5i, _add8r, _add8i, &_pow9r, &_pow9i);
        double _add10r = 0, _add10i = 0;
        _add10r = _pow4r + _pow9r; _add10i = _pow4i + _pow9i;
        double _mul11r = 0, _mul11i = 0;
        c_mul(j, 0, M_PI, 0, &_mul11r, &_mul11i);
        double _c12r = 0, _c12i = 0;
        _c12r = 1.0; _c12i = 0;
        double _add13r = 0, _add13i = 0;
        _add13r = 0 + _c12r; _add13i = 0 + _c12i;
        double _div14r = 0, _div14i = 0;
        c_div(_mul11r, _mul11i, _add13r, _add13i, &_div14r, &_div14i);
        double _sin15r = 0, _sin15i = 0;
        c_sin(_div14r, _div14i, &_sin15r, &_sin15i);
        double _add16r = 0, _add16i = 0;
        _add16r = _add10r + _sin15r; _add16i = _add10i + _sin15i;
        double mag_r = _add16r, mag_i = _add16i;
        double _ang17r = 0, _ang17i = 0;
        _ang17r = c_arg(x1r, x1i); _ang17i = 0;
        double _c18r = 0, _c18i = 0;
        _c18r = 1.0; _c18i = 0;
        double _add19r = 0, _add19i = 0;
        _add19r = j + _c18r; _add19i = 0 + _c18i;
        double _log20r = 0, _log20i = 0;
        c_log(_add19r, _add19i, &_log20r, &_log20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_ang17r, _ang17i, _log20r, _log20i, &_mul21r, &_mul21i);
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x2r, x2i); _ang22i = 0;
        double _atan23r = 0, _atan23i = 0;
        c_atan(j, 0, &_atan23r, &_atan23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_ang22r, _ang22i, _atan23r, _atan23i, &_mul24r, &_mul24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _mul21r + _mul24r; _add25i = _mul21i + _mul24i;
        double _mul26r = 0, _mul26i = 0;
        c_mul(0, 0, M_PI, 0, &_mul26r, &_mul26i);
        double _div27r = 0, _div27i = 0;
        c_div(_mul26r, _mul26i, j, 0, &_div27r, &_div27i);
        double _cos28r = 0, _cos28i = 0;
        c_cos(_div27r, _div27i, &_cos28r, &_cos28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _add25r + _cos28r; _add29i = _add25i + _cos28i;
        double ang_r = _add29r, ang_i = _add29i;
        double _cos30r = 0, _cos30i = 0;
        c_cos(ang_r, ang_i, &_cos30r, &_cos30i);
        double _c31r = 0, _c31i = 0;
        _c31r = 0.0; _c31i = 1.0;
        double _sin32r = 0, _sin32i = 0;
        c_sin(ang_r, ang_i, &_sin32r, &_sin32i);
        double _mul33r = 0, _mul33i = 0;
        c_mul(_c31r, _c31i, _sin32r, _sin32i, &_mul33r, &_mul33i);
        double _add34r = 0, _add34i = 0;
        _add34r = _cos30r + _mul33r; _add34i = _cos30i + _mul33i;
        double _mul35r = 0, _mul35i = 0;
        c_mul(mag_r, mag_i, _add34r, _add34i, &_mul35r, &_mul35i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul35r; cIm[_idx] = _mul35i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_300_c(double x1r, double x1i, double x2r, double x2i,
                     const double *cfpv, int n_cfpv,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 35;
    (void)cfpv; (void)n_cfpv;
    for (int _i = 0; _i < 35; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double n = 35.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 1.0; _c1i = 0;
    double _add2r = 0, _add2i = 0;
    _add2r = n + _c1r; _add2i = 0 + _c1i;
    for (int j = 1; j < (int)(_add2r); j++) {
        double _re3r = 0, _re3i = 0;
        _re3r = x1r; _re3i = 0;
        double r1_r = _re3r, r1_i = _re3i;
        double _re4r = 0, _re4i = 0;
        _re4r = x2r; _re4i = 0;
        double r2_r = _re4r, r2_i = _re4i;
        double _im5r = 0, _im5i = 0;
        _im5r = x1i; _im5i = 0;
        double i1_r = _im5r, i1_i = _im5i;
        double _im6r = 0, _im6i = 0;
        _im6r = x2i; _im6i = 0;
        double i2_r = _im6r, i2_i = _im6i;
        double _abs7r = 0, _abs7i = 0;
        _abs7r = c_abs(x1r, x1i); _abs7i = 0;
        double _add8r = 0, _add8i = 0;
        _add8r = _abs7r + j; _add8i = _abs7i + 0;
        double _log9r = 0, _log9i = 0;
        c_log(_add8r, _add8i, &_log9r, &_log9i);
        double _mul10r = 0, _mul10i = 0;
        c_mul(r1_r, r1_i, j, 0, &_mul10r, &_mul10i);
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = j + _c11r; _add12i = 0 + _c11i;
        double _div13r = 0, _div13i = 0;
        c_div(i2_r, i2_i, _add12r, _add12i, &_div13r, &_div13i);
        double _sub14r = 0, _sub14i = 0;
        _sub14r = _mul10r - _div13r; _sub14i = _mul10i - _div13i;
        double _abs15r = 0, _abs15i = 0;
        _abs15r = c_abs(_sub14r, _sub14i); _abs15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(_log9r, _log9i, _abs15r, _abs15i, &_mul16r, &_mul16i);
        double _prod17r = 0, _prod17i = 0;
        c_mul(r1_r, r1_i, i2_r, i2_i, &_prod17r, &_prod17i);
        double _prod18r = 0, _prod18i = 0;
        c_mul(_prod17r, _prod17i, j, 0, &_prod18r, &_prod18i);
        double _add19r = 0, _add19i = 0;
        _add19r = _mul16r + _prod18r; _add19i = _mul16i + _prod18i;
        double term_mag_r = _add19r, term_mag_i = _add19i;
        double _ang20r = 0, _ang20i = 0;
        _ang20r = c_arg(x1r, x1i); _ang20i = 0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(_ang20r, _ang20i, j, 0, &_mul21r, &_mul21i);
        double _ang22r = 0, _ang22i = 0;
        _ang22r = c_arg(x2r, x2i); _ang22i = 0;
        double _sub23r = 0, _sub23i = 0;
        _sub23r = n - j; _sub23i = 0 - 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(_ang22r, _ang22i, _sub23r, _sub23i, &_mul24r, &_mul24i);
        double _sub25r = 0, _sub25i = 0;
        _sub25r = _mul21r - _mul24r; _sub25i = _mul21i - _mul24i;
        double _mul26r = 0, _mul26i = 0;
        c_mul(j, 0, r2_r, r2_i, &_mul26r, &_mul26i);
        double _sin27r = 0, _sin27i = 0;
        c_sin(_mul26r, _mul26i, &_sin27r, &_sin27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(j, 0, i1_r, i1_i, &_mul28r, &_mul28i);
        double _cos29r = 0, _cos29i = 0;
        c_cos(_mul28r, _mul28i, &_cos29r, &_cos29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_sin27r, _sin27i, _cos29r, _cos29i, &_mul30r, &_mul30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _sub25r + _mul30r; _add31i = _sub25i + _mul30i;
        double term_angle_r = _add31r, term_angle_i = _add31i;
        double _cos32r = 0, _cos32i = 0;
        c_cos(term_angle_r, term_angle_i, &_cos32r, &_cos32i);
        double _c33r = 0, _c33i = 0;
        _c33r = 0.0; _c33i = 1.0;
        double _sin34r = 0, _sin34i = 0;
        c_sin(term_angle_r, term_angle_i, &_sin34r, &_sin34i);
        double _mul35r = 0, _mul35i = 0;
        c_mul(_c33r, _c33i, _sin34r, _sin34i, &_mul35r, &_mul35i);
        double _add36r = 0, _add36i = 0;
        _add36r = _cos32r + _mul35r; _add36i = _cos32i + _mul35i;
        double _mul37r = 0, _mul37i = 0;
        c_mul(term_mag_r, term_mag_i, _add36r, _add36i, &_mul37r, &_mul37i);
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 35) { cRe[_idx] = _mul37r; cIm[_idx] = _mul37i; } }
    }
    for (int _i = 0; _i < 35; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}


