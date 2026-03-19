/* AUTO-GENERATED from poly900.py — do not edit manually */
/* 76 coefficient functions */

static void poly_801_c(double x1r, double x1i, double x2r, double x2i,
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
        c_powr(_abs46r, _abs46i, _div48r, &_pow49r, &_pow49i);
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
        c_powr(_abs54r, _abs54i, _div56r, &_pow57r, &_pow57i);
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

static void poly_802_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int _si = 0; _si < 3; _si++) {
        int _si_idx = _si + 0;
        double _unk1r = 0, _unk1i = 0;
        /* WARNING: unhandled node List(elts=[BinOp(left=BinOp(left=Constant(value=2), op=Mult(), right=Name(id='t1', ctx=Load())), op=Add(), right=BinOp(left=Constant(value=3), op=Mult(), right=Name(id='t2', ctx=Load()))), BinOp(left=BinOp(left=Constant(value=3), op=Mult(), right=Name(id='t1', ctx=Load())), op=Sub(), right=BinOp(left=Constant(value=2), op=Mult(), right=Name(id='t2', ctx=Load()))), BinOp(left=BinOp(left=Name(id='t1', ctx=Load()), op=Pow(), right=Constant(value=2)), op=Sub(), right=BinOp(left=Name(id='t2', ctx=Load()), op=Pow(), right=Constant(value=2)))], ctx=Load()) */
        cRe[_si_idx] = _unk1r; cIm[_si_idx] = _unk1i;
    }
    for (int _si = 0; _si < 2; _si++) {
        int _si_idx = _si + 3;
        double _unk2r = 0, _unk2i = 0;
        /* WARNING: unhandled node List(elts=[Call(func=Attribute(value=Name(id='np', ctx=Load()), attr='real', ctx=Load()), args=[BinOp(left=Name(id='t1', ctx=Load()), op=Mult(), right=Name(id='t2', ctx=Load()))]), Call(func=Attribute(value=Name(id='np', ctx=Load()), attr='imag', ctx=Load()), args=[BinOp(left=Name(id='t1', ctx=Load()), op=Mult(), right=Name(id='t2', ctx=Load()))])], ctx=Load()) */
        cRe[_si_idx] = _unk2r; cIm[_si_idx] = _unk2i;
    }
    for (int k = 5; k < 20; k++) {
        double _cf3r = 0, _cf3i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 25) { _cf3r = cRe[_idx]; _cf3i = cIm[_idx]; } }
        double _sin4r = 0, _sin4i = 0;
        c_sin(_cf3r, _cf3i, &_sin4r, &_sin4i);
        double _cf5r = 0, _cf5i = 0;
        { int _idx = (k - 2); if (_idx >= 0 && _idx < 25) { _cf5r = cRe[_idx]; _cf5i = cIm[_idx]; } }
        double _cos6r = 0, _cos6i = 0;
        c_cos(_cf5r, _cf5i, &_cos6r, &_cos6i);
        double _add7r = 0, _add7i = 0;
        _add7r = _sin4r + _cos6r; _add7i = _sin4i + _cos6i;
        { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add7r; cIm[_idx] = _add7i; } }
        double _cf8r = 0, _cf8i = 0;
        { int _idx = k; if (_idx >= 0 && _idx < 25) { _cf8r = cRe[_idx]; _cf8i = cIm[_idx]; } }
        double _abs9r = 0, _abs9i = 0;
        _abs9r = c_abs(_cf8r, _cf8i); _abs9i = 0;
        double mod_cf = _abs9r; /* +_abs9ii */
        double _c10r = 0, _c10i = 0;
        _c10r = 0.0; _c10i = 0;
        if (mod_cf != _c10r) {
            double _cf11r = 0, _cf11i = 0;
            { int _idx = k; if (_idx >= 0 && _idx < 25) { _cf11r = cRe[_idx]; _cf11i = cIm[_idx]; } }
            double _div12r = 0, _div12i = 0;
            c_div(_cf11r, _cf11i, mod_cf, 0, &_div12r, &_div12i);
            { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _div12r; cIm[_idx] = _div12i; } }
        } else {
            double _c13r = 0, _c13i = 0;
            _c13r = 1.0; _c13i = 0;
            { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _c13r; cIm[_idx] = _c13i; } }
        }
    }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_803_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _add1r = 0, _add1i = 0;
    _add1r = x1r + x2r; _add1i = x1i + x2i;
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add1r; cIm[_idx] = _add1i; } }
    double _abs2r = 0, _abs2i = 0;
    _abs2r = c_abs(x1r, x1i); _abs2i = 0;
    double _abs3r = 0, _abs3i = 0;
    _abs3r = c_abs(x2r, x2i); _abs3i = 0;
    double _mul4r = 0, _mul4i = 0;
    c_mul(_abs2r, _abs2i, _abs3r, _abs3i, &_mul4r, &_mul4i);
    { int _idx = 1; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul4r; cIm[_idx] = _mul4i; } }
    double _mul5r = 0, _mul5i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul5r, &_mul5i);
    double _re6r = 0, _re6i = 0;
    _re6r = _mul5r; _re6i = 0;
    { int _idx = 2; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _re6r; cIm[_idx] = _re6i; } }
    double _mul7r = 0, _mul7i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul7r, &_mul7i);
    double _im8r = 0, _im8i = 0;
    _im8r = _mul7i; _im8i = 0;
    { int _idx = 3; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _im8r; cIm[_idx] = _im8i; } }
    double _add9r = 0, _add9i = 0;
    _add9r = x1r + x2r; _add9i = x1i + x2i;
    double _ang10r = 0, _ang10i = 0;
    _ang10r = c_arg(_add9r, _add9i); _ang10i = 0;
    { int _idx = 4; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _ang10r; cIm[_idx] = _ang10i; } }
    for (int k = 5; k < 26; k++) {
        double _cf11r = 0, _cf11i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 25) { _cf11r = cRe[_idx]; _cf11i = cIm[_idx]; } }
        double _mul12r = 0, _mul12i = 0;
        c_mul(k, 0, x1r, x1i, &_mul12r, &_mul12i);
        double _cos13r = 0, _cos13i = 0;
        c_cos(_mul12r, _mul12i, &_cos13r, &_cos13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _cf11r + _cos13r; _add14i = _cf11i + _cos13i;
        double _abs15r = 0, _abs15i = 0;
        _abs15r = c_abs(x2r, x2i); _abs15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(k, 0, _abs15r, _abs15i, &_mul16r, &_mul16i);
        double _sin17r = 0, _sin17i = 0;
        c_sin(_mul16r, _mul16i, &_sin17r, &_sin17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 0.0; _c18i = 1.0;
        double _mul19r = 0, _mul19i = 0;
        c_mul(_sin17r, _sin17i, _c18r, _c18i, &_mul19r, &_mul19i);
        double _add20r = 0, _add20i = 0;
        _add20r = _add14r + _mul19r; _add20i = _add14i + _mul19i;
        { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add20r; cIm[_idx] = _add20i; } }
    }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_804_c(double x1r, double x1i, double x2r, double x2i,
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

static void poly_805_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
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
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add5r; cIm[_idx] = _add5i; } }
    double _mul6r = 0, _mul6i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul6r, &_mul6i);
    { int _idx = 1; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul6r; cIm[_idx] = _mul6i; } }
    for (int _si = 0; _si < 6; _si++) {
        int _si_idx = _si + 2;
        double _arange7r = 0, _arange7i = 0;
        _arange7r = (double)(_si + 1); _arange7i = 0;
        double _pow8r = 0, _pow8i = 0;
        c_powr(x2r, x2i, 0, &_pow8r, &_pow8i);
        double _smul9r = 0, _smul9i = 0;
        c_mul(x1r, x1i, _pow8r, _pow8i, &_smul9r, &_smul9i);
        cRe[_si_idx] = _smul9r; cIm[_si_idx] = _smul9i;
    }
    for (int _si = 0; _si < 6; _si++) {
        int _si_idx = _si + 8;
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
        double _arange15r = 0, _arange15i = 0;
        _arange15r = (double)(_si + 1); _arange15i = 0;
        double _sdiv16r = 0, _sdiv16i = 0;
        c_div(_add14r, _add14i, _arange15r, _arange15i, &_sdiv16r, &_sdiv16i);
        cRe[_si_idx] = _sdiv16r; cIm[_si_idx] = _sdiv16i;
    }
    for (int _si = 0; _si < 6; _si++) {
        int _si_idx = _si + 14;
        double _c17r = 0, _c17i = 0;
        _c17r = 3.0; _c17i = 0;
        double _pow18r = 0, _pow18i = 0;
        c_mul(x1r, x1i, x1r, x1i, &_pow18r, &_pow18i);
        c_mul(_pow18r, _pow18i, x1r, x1i, &_pow18r, &_pow18i);
        double _arange19r = 0, _arange19i = 0;
        _arange19r = (double)(_si + 1); _arange19i = 0;
        double _pow20r = 0, _pow20i = 0;
        c_powr(x2r, x2i, 0, &_pow20r, &_pow20i);
        double _smul21r = 0, _smul21i = 0;
        c_mul(_pow18r, _pow18i, _pow20r, _pow20i, &_smul21r, &_smul21i);
        cRe[_si_idx] = _smul21r; cIm[_si_idx] = _smul21i;
    }
    for (int _si = 0; _si < 5; _si++) {
        int _si_idx = _si + 20;
        double _add22r = 0, _add22i = 0;
        _add22r = x1r + x2r; _add22i = x1i + x2i;
        double _arange23r = 0, _arange23i = 0;
        _arange23r = (double)(_si + 1); _arange23i = 0;
        double _add24r = 0, _add24i = 0;
        _add24r = x1r + x2r; _add24i = x1i + x2i;
        double _pow25r = 0, _pow25i = 0;
        c_powr(_add24r, _add24i, 0, &_pow25r, &_pow25i);
        cRe[_si_idx] = _pow25r; cIm[_si_idx] = _pow25i;
    }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_806: too complex for auto-transpile, stubbed */
static void poly_806_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_807_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _mul1r = 0, _mul1i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul1r, &_mul1i);
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul1r; cIm[_idx] = _mul1i; } }
    double _c2r = 0, _c2i = 0;
    _c2r = 2.0; _c2i = 0;
    double _pow3r = 0, _pow3i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow3r, &_pow3i);
    double _c4r = 0, _c4i = 0;
    _c4r = 2.0; _c4i = 0;
    double _c5r = 0, _c5i = 0;
    _c5r = 2.0; _c5i = 0;
    double _pow6r = 0, _pow6i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow6r, &_pow6i);
    double _mul7r = 0, _mul7i = 0;
    c_mul(_c4r, _c4i, _pow6r, _pow6i, &_mul7r, &_mul7i);
    double _add8r = 0, _add8i = 0;
    _add8r = _pow3r + _mul7r; _add8i = _pow3i + _mul7i;
    { int _idx = 1; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add8r; cIm[_idx] = _add8i; } }
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
    { int _idx = 2; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _sub13r; cIm[_idx] = _sub13i; } }
    for (int k = 3; k < 14; k++) {
        double _mul14r = 0, _mul14i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul14r, &_mul14i);
        double _c15r = 0, _c15i = 0;
        _c15r = 1.0; _c15i = 0;
        double _add16r = 0, _add16i = 0;
        _add16r = k + _c15r; _add16i = 0 + _c15i;
        double _div17r = 0, _div17i = 0;
        c_div(_mul14r, _mul14i, _add16r, _add16i, &_div17r, &_div17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 0.0; _c18i = 1.0;
        double _cf19r = 0, _cf19i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 25) { _cf19r = cRe[_idx]; _cf19i = cIm[_idx]; } }
        double _mul20r = 0, _mul20i = 0;
        c_mul(_c18r, _c18i, _cf19r, _cf19i, &_mul20r, &_mul20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _div17r + _mul20r; _add21i = _div17i + _mul20i;
        { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add21r; cIm[_idx] = _add21i; } }
    }
    double _cf22r = 0, _cf22i = 0;
    { int _idx = 13; if (_idx >= 0 && _idx < 25) { _cf22r = cRe[_idx]; _cf22i = cIm[_idx]; } }
    double _attr23r = 0, _attr23i = 0;
    _attr23r = _cf22r; _attr23i = 0;
    double _cf24r = 0, _cf24i = 0;
    { int _idx = 9; if (_idx >= 0 && _idx < 25) { _cf24r = cRe[_idx]; _cf24i = cIm[_idx]; } }
    double _attr25r = 0, _attr25i = 0;
    _attr25r = _cf24i; _attr25i = 0;
    double _add26r = 0, _add26i = 0;
    _add26r = _attr23r + _attr25r; _add26i = _attr23i + _attr25i;
    { int _idx = 14; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add26r; cIm[_idx] = _add26i; } }
    for (int k = 15; k < 25; k++) {
        double _cf27r = 0, _cf27i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 25) { _cf27r = cRe[_idx]; _cf27i = cIm[_idx]; } }
        double _c28r = 0, _c28i = 0;
        _c28r = 0.0; _c28i = 1.0;
        double _c29r = 0, _c29i = 0;
        _c29r = 2.0; _c29i = 0;
        double _pow30r = 0, _pow30i = 0;
        c_mul(x2r, x2i, x2r, x2i, &_pow30r, &_pow30i);
        double _mul31r = 0, _mul31i = 0;
        c_mul(_c28r, _c28i, _pow30r, _pow30i, &_mul31r, &_mul31i);
        double _add32r = 0, _add32i = 0;
        _add32r = _cf27r + _mul31r; _add32i = _cf27i + _mul31i;
        double _ang33r = 0, _ang33i = 0;
        _ang33r = c_arg(_add32r, _add32i); _ang33i = 0;
        double arg_val = _ang33r; /* +_ang33ii */
        double _flr34r = 0, _flr34i = 0;
        _flr34r = floor(arg_val); _flr34i = 0;
        { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _flr34r; cIm[_idx] = _flr34i; } }
    }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_808_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 0; k < 25; k++) {
        double _add1r = 0, _add1i = 0;
        _add1r = k + x1r; _add1i = 0 + x1i;
        double _add2r = 0, _add2i = 0;
        _add2r = k + x2r; _add2i = 0 + x2i;
        double _div3r = 0, _div3i = 0;
        c_div(_add1r, _add1i, _add2r, _add2i, &_div3r, &_div3i);
        { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _div3r; cIm[_idx] = _div3i; } }
    }
    double _add4r = 0, _add4i = 0;
    _add4r = x1r + x2r; _add4i = x1i + x2i;
    double _abs5r = 0, _abs5i = 0;
    _abs5r = c_abs(_add4r, _add4i); _abs5i = 0;
    double _log6r = 0, _log6i = 0;
    c_log(_abs5r, _abs5i, &_log6r, &_log6i);
    cRe[4] += _log6r; cIm[4] += _log6i;
    double _re7r = 0, _re7i = 0;
    _re7r = x1r; _re7i = 0;
    double _sin8r = 0, _sin8i = 0;
    c_sin(_re7r, _re7i, &_sin8r, &_sin8i);
    double _im9r = 0, _im9i = 0;
    _im9r = x2i; _im9i = 0;
    double _cos10r = 0, _cos10i = 0;
    c_cos(_im9r, _im9i, &_cos10r, &_cos10i);
    double _add11r = 0, _add11i = 0;
    _add11r = _sin8r + _cos10r; _add11i = _sin8i + _cos10i;
    cRe[9] += _add11r; cIm[9] += _add11i;
    double _cf12r = 0, _cf12i = 0;
    { int _idx = 13; if (_idx >= 0 && _idx < 25) { _cf12r = cRe[_idx]; _cf12i = cIm[_idx]; } }
    double _abs13r = 0, _abs13i = 0;
    _abs13r = c_abs(_cf12r, _cf12i); _abs13i = 0;
    double _c14r = 0, _c14i = 0;
    _c14r = 2.0; _c14i = 0;
    double _pow15r = 0, _pow15i = 0;
    c_mul(_abs13r, _abs13i, _abs13r, _abs13i, &_pow15r, &_pow15i);
    double _cf16r = 0, _cf16i = 0;
    { int _idx = 12; if (_idx >= 0 && _idx < 25) { _cf16r = cRe[_idx]; _cf16i = cIm[_idx]; } }
    double _ang17r = 0, _ang17i = 0;
    _ang17r = c_arg(_cf16r, _cf16i); _ang17i = 0;
    double _c18r = 0, _c18i = 0;
    _c18r = 2.0; _c18i = 0;
    double _pow19r = 0, _pow19i = 0;
    c_mul(_ang17r, _ang17i, _ang17r, _ang17i, &_pow19r, &_pow19i);
    double _add20r = 0, _add20i = 0;
    _add20r = _pow15r + _pow19r; _add20i = _pow15i + _pow19i;
    cRe[14] += _add20r; cIm[14] += _add20i;
    double _re21r = 0, _re21i = 0;
    _re21r = x2r; _re21i = 0;
    double _im22r = 0, _im22i = 0;
    _im22r = x1i; _im22i = 0;
    double _mul23r = 0, _mul23i = 0;
    c_mul(_re21r, _re21i, _im22r, _im22i, &_mul23r, &_mul23i);
    double _abs24r = 0, _abs24i = 0;
    _abs24r = c_abs(_mul23r, _mul23i); _abs24i = 0;
    cRe[19] += _abs24r; cIm[19] += _abs24i;
    double _conj25r = 0, _conj25i = 0;
    _conj25r = x2r; _conj25i = -(x2i);
    double _add26r = 0, _add26i = 0;
    _add26r = x1r + _conj25r; _add26i = x1i + _conj25i;
    double _abs27r = 0, _abs27i = 0;
    _abs27r = c_abs(_add26r, _add26i); _abs27i = 0;
    cRe[24] += _abs27r; cIm[24] += _abs27i;
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_809_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _add1r = 0, _add1i = 0;
    _add1r = x1r + x2r; _add1i = x1i + x2i;
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add1r; cIm[_idx] = _add1i; } }
    for (int k = 1; k < 25; k++) {
        double _cf2r = 0, _cf2i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 25) { _cf2r = cRe[_idx]; _cf2i = cIm[_idx]; } }
        double _ang3r = 0, _ang3i = 0;
        _ang3r = c_arg(_cf2r, _cf2i); _ang3i = 0;
        double _mul4r = 0, _mul4i = 0;
        c_mul(k, 0, _ang3r, _ang3i, &_mul4r, &_mul4i);
        double _cos5r = 0, _cos5i = 0;
        c_cos(_mul4r, _mul4i, &_cos5r, &_cos5i);
        double _abs6r = 0, _abs6i = 0;
        _abs6r = c_abs(x1r, x1i); _abs6i = 0;
        double _mul7r = 0, _mul7i = 0;
        c_mul(k, 0, _abs6r, _abs6i, &_mul7r, &_mul7i);
        double _sin8r = 0, _sin8i = 0;
        c_sin(_mul7r, _mul7i, &_sin8r, &_sin8i);
        double _add9r = 0, _add9i = 0;
        _add9r = _cos5r + _sin8r; _add9i = _cos5i + _sin8i;
        double _conj10r = 0, _conj10i = 0;
        _conj10r = x2r; _conj10i = -(x2i);
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _add12r = 0, _add12i = 0;
        _add12r = _c11r + x1r; _add12i = _c11i + x1i;
        double _abs13r = 0, _abs13i = 0;
        _abs13r = c_abs(_add12r, _add12i); _abs13i = 0;
        double _div14r = 0, _div14i = 0;
        c_div(_conj10r, _conj10i, _abs13r, _abs13i, &_div14r, &_div14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _add9r + _div14r; _add15i = _add9i + _div14i;
        { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add15r; cIm[_idx] = _add15i; } }
        double _np16r = 0, _np16i = 0;
        /* WARNING: unhandled np.isinf */
        double _np17r = 0, _np17i = 0;
        /* WARNING: unhandled np.isnan */
        if (((_np16r != 0 || _np16i != 0)) || ((_np17r != 0 || _np17i != 0))) {
            double _cf18r = 0, _cf18i = 0;
            { int _idx = (k - 1); if (_idx >= 0 && _idx < 25) { _cf18r = cRe[_idx]; _cf18i = cIm[_idx]; } }
            { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _cf18r; cIm[_idx] = _cf18i; } }
        }
    }
    double _cf19r = 0, _cf19i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { _cf19r = cRe[_idx]; _cf19i = cIm[_idx]; } }
    double _c20r = 0, _c20i = 0;
    _c20r = 3.0; _c20i = 0;
    double _pow21r = 0, _pow21i = 0;
    c_mul(_cf19r, _cf19i, _cf19r, _cf19i, &_pow21r, &_pow21i);
    c_mul(_pow21r, _pow21i, _cf19r, _cf19i, &_pow21r, &_pow21i);
    double _cf22r = 0, _cf22i = 0;
    { int _idx = 1; if (_idx >= 0 && _idx < 25) { _cf22r = cRe[_idx]; _cf22i = cIm[_idx]; } }
    double _c23r = 0, _c23i = 0;
    _c23r = 2.0; _c23i = 0;
    double _pow24r = 0, _pow24i = 0;
    c_mul(_cf22r, _cf22i, _cf22r, _cf22i, &_pow24r, &_pow24i);
    double _add25r = 0, _add25i = 0;
    _add25r = _pow21r + _pow24r; _add25i = _pow21i + _pow24i;
    double _cf26r = 0, _cf26i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { _cf26r = cRe[_idx]; _cf26i = cIm[_idx]; } }
    double _cf27r = 0, _cf27i = 0;
    { int _idx = 1; if (_idx >= 0 && _idx < 25) { _cf27r = cRe[_idx]; _cf27i = cIm[_idx]; } }
    double _mul28r = 0, _mul28i = 0;
    c_mul(_cf26r, _cf26i, _cf27r, _cf27i, &_mul28r, &_mul28i);
    double _sub29r = 0, _sub29i = 0;
    _sub29r = _add25r - _mul28r; _sub29i = _add25i - _mul28i;
    { int _idx = 9; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _sub29r; cIm[_idx] = _sub29i; } }
    double _np30r = 0, _np30i = 0;
    /* WARNING: unhandled np.isinf */
    double _np31r = 0, _np31i = 0;
    /* WARNING: unhandled np.isnan */
    if (((_np30r != 0 || _np30i != 0)) || ((_np31r != 0 || _np31i != 0))) {
        double _cf32r = 0, _cf32i = 0;
        { int _idx = 8; if (_idx >= 0 && _idx < 25) { _cf32r = cRe[_idx]; _cf32i = cIm[_idx]; } }
        { int _idx = 9; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _cf32r; cIm[_idx] = _cf32i; } }
    }
    double _cf33r = 0, _cf33i = 0;
    { int _idx = 13; if (_idx >= 0 && _idx < 25) { _cf33r = cRe[_idx]; _cf33i = cIm[_idx]; } }
    double _abs34r = 0, _abs34i = 0;
    _abs34r = c_abs(_cf33r, _cf33i); _abs34i = 0;
    double _log35r = 0, _log35i = 0;
    c_log(_abs34r, _abs34i, &_log35r, &_log35i);
    double _c36r = 0, _c36i = 0;
    _c36r = 2.0; _c36i = 0;
    double _pow37r = 0, _pow37i = 0;
    c_mul(x1r, x1i, x1r, x1i, &_pow37r, &_pow37i);
    double _sub38r = 0, _sub38i = 0;
    _sub38r = _log35r - _pow37r; _sub38i = _log35i - _pow37i;
    double _c39r = 0, _c39i = 0;
    _c39r = 2.0; _c39i = 0;
    double _pow40r = 0, _pow40i = 0;
    c_mul(x2r, x2i, x2r, x2i, &_pow40r, &_pow40i);
    double _add41r = 0, _add41i = 0;
    _add41r = _sub38r + _pow40r; _add41i = _sub38i + _pow40i;
    { int _idx = 14; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add41r; cIm[_idx] = _add41i; } }
    double _np42r = 0, _np42i = 0;
    /* WARNING: unhandled np.isinf */
    double _np43r = 0, _np43i = 0;
    /* WARNING: unhandled np.isnan */
    if (((_np42r != 0 || _np42i != 0)) || ((_np43r != 0 || _np43i != 0))) {
        double _cf44r = 0, _cf44i = 0;
        { int _idx = 13; if (_idx >= 0 && _idx < 25) { _cf44r = cRe[_idx]; _cf44i = cIm[_idx]; } }
        { int _idx = 14; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _cf44r; cIm[_idx] = _cf44i; } }
    }
    double _cf45r = 0, _cf45i = 0;
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { _cf45r = cRe[_idx]; _cf45i = cIm[_idx]; } }
    double _add46r = 0, _add46i = 0;
    _add46r = x1r + x2r; _add46i = x1i + x2i;
    double _c47r = 0, _c47i = 0;
    _c47r = 2.0; _c47i = 0;
    double _pow48r = 0, _pow48i = 0;
    c_mul(_add46r, _add46i, _add46r, _add46i, &_pow48r, &_pow48i);
    double _mul49r = 0, _mul49i = 0;
    c_mul(_cf45r, _cf45i, _pow48r, _pow48i, &_mul49r, &_mul49i);
    double _cf50r = 0, _cf50i = 0;
    { int _idx = 2; if (_idx >= 0 && _idx < 25) { _cf50r = cRe[_idx]; _cf50i = cIm[_idx]; } }
    double _c51r = 0, _c51i = 0;
    _c51r = 1.0; _c51i = 0;
    double _mul52r = 0, _mul52i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul52r, &_mul52i);
    double _abs53r = 0, _abs53i = 0;
    _abs53r = c_abs(_mul52r, _mul52i); _abs53i = 0;
    double _add54r = 0, _add54i = 0;
    _add54r = _c51r + _abs53r; _add54i = _c51i + _abs53i;
    double _div55r = 0, _div55i = 0;
    c_div(_cf50r, _cf50i, _add54r, _add54i, &_div55r, &_div55i);
    double _sub56r = 0, _sub56i = 0;
    _sub56r = _mul49r - _div55r; _sub56i = _mul49i - _div55i;
    { int _idx = 19; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _sub56r; cIm[_idx] = _sub56i; } }
    double _np57r = 0, _np57i = 0;
    /* WARNING: unhandled np.isinf */
    double _np58r = 0, _np58i = 0;
    /* WARNING: unhandled np.isnan */
    if (((_np57r != 0 || _np57i != 0)) || ((_np58r != 0 || _np58i != 0))) {
        double _cf59r = 0, _cf59i = 0;
        { int _idx = 18; if (_idx >= 0 && _idx < 25) { _cf59r = cRe[_idx]; _cf59i = cIm[_idx]; } }
        { int _idx = 19; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _cf59r; cIm[_idx] = _cf59i; } }
    }
    double _add60r = 0, _add60i = 0;
    _add60r = x1r + x2r; _add60i = x1i + x2i;
    double _c61r = 0, _c61i = 0;
    _c61r = 3.0; _c61i = 0;
    double _pow62r = 0, _pow62i = 0;
    c_mul(_add60r, _add60i, _add60r, _add60i, &_pow62r, &_pow62i);
    c_mul(_pow62r, _pow62i, _add60r, _add60i, &_pow62r, &_pow62i);
    double _cf63r = 0, _cf63i = 0;
    { int _idx = 23; if (_idx >= 0 && _idx < 25) { _cf63r = cRe[_idx]; _cf63i = cIm[_idx]; } }
    double _sub64r = 0, _sub64i = 0;
    _sub64r = _pow62r - _cf63r; _sub64i = _pow62i - _cf63i;
    { int _idx = 24; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _sub64r; cIm[_idx] = _sub64i; } }
    double _np65r = 0, _np65i = 0;
    /* WARNING: unhandled np.isinf */
    double _np66r = 0, _np66i = 0;
    /* WARNING: unhandled np.isnan */
    if (((_np65r != 0 || _np65i != 0)) || ((_np66r != 0 || _np66i != 0))) {
        double _cf67r = 0, _cf67i = 0;
        { int _idx = 23; if (_idx >= 0 && _idx < 25) { _cf67r = cRe[_idx]; _cf67i = cIm[_idx]; } }
        { int _idx = 24; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _cf67r; cIm[_idx] = _cf67i; } }
    }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_810_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int i = 0; i < 25; i++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 2.0; _c1i = 0;
        double _mod2r = 0, _mod2i = 0;
        _mod2r = fmod(i, _c1r); _mod2i = 0;
        double _c3r = 0, _c3i = 0;
        _c3r = 1.0; _c3i = 0;
        if (_mod2r == _c3r) {
            double _mul4r = 0, _mul4i = 0;
            c_mul(i, 0, x1r, x1i, &_mul4r, &_mul4i);
            double _c5r = 0, _c5i = 0;
            _c5r = 3.0; _c5i = 0;
            double _mul6r = 0, _mul6i = 0;
            c_mul(_c5r, _c5i, i, 0, &_mul6r, &_mul6i);
            double _mul7r = 0, _mul7i = 0;
            c_mul(_mul6r, _mul6i, x2r, x2i, &_mul7r, &_mul7i);
            double _add8r = 0, _add8i = 0;
            _add8r = _mul4r + _mul7r; _add8i = _mul4i + _mul7i;
            double _c9r = 0, _c9i = 0;
            _c9r = 1.0; _c9i = 0;
            double _add10r = 0, _add10i = 0;
            _add10r = i + _c9r; _add10i = 0 + _c9i;
            double _c11r = 0, _c11i = 0;
            _c11r = 2.0; _c11i = 0;
            double _pow12r = 0, _pow12i = 0;
            c_mul(_add10r, _add10i, _add10r, _add10i, &_pow12r, &_pow12i);
            double _div13r = 0, _div13i = 0;
            c_div(_add8r, _add8i, _pow12r, _pow12i, &_div13r, &_div13i);
            double _pow14r = 0, _pow14i = 0;
            c_powr(_div13r, _div13i, i, &_pow14r, &_pow14i);
            { int _idx = i; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _pow14r; cIm[_idx] = _pow14i; } }
        } else {
            double _conj15r = 0, _conj15i = 0;
            _conj15r = x2r; _conj15i = -(x2i);
            double _add16r = 0, _add16i = 0;
            _add16r = x1r + _conj15r; _add16i = x1i + _conj15i;
            double _pow17r = 0, _pow17i = 0;
            c_powr(_add16r, _add16i, i, &_pow17r, &_pow17i);
            { int _idx = i; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _pow17r; cIm[_idx] = _pow17i; } }
        }
    }
    double _unk18r = 0, _unk18i = 0;
    /* WARNING: unhandled node Compare(left=Name(id='cf', ctx=Load()), ops=[Eq()], comparators=[Attribute(value=Name(id='np', ctx=Load()), attr='inf', ctx=Load())]) */
    double _c19r = 0, _c19i = 0;
    _c19r = 10000000000.0; _c19i = 0;
    { int _idx = (int)(_unk18r); if (_idx >= 0 && _idx < 25) { cRe[_idx] = _c19r; cIm[_idx] = _c19i; } }
    double _unk20r = 0, _unk20i = 0;
    /* WARNING: unhandled node Compare(left=Name(id='cf', ctx=Load()), ops=[Eq()], comparators=[UnaryOp(op=USub(), operand=Attribute(value=Name(id='np', ctx=Load()), attr='inf', ctx=Load()))]) */
    double _c21r = 0, _c21i = 0;
    _c21r = 10000000000.0; _c21i = 0;
    double _neg22r = 0, _neg22i = 0;
    _neg22r = -(_c21r); _neg22i = -(_c21i);
    { int _idx = (int)(_unk20r); if (_idx >= 0 && _idx < 25) { cRe[_idx] = _neg22r; cIm[_idx] = _neg22i; } }
    double _np23r = 0, _np23i = 0;
    /* WARNING: unhandled np.isnan */
    double _c24r = 0, _c24i = 0;
    _c24r = 0.0; _c24i = 0;
    { int _idx = (int)(_np23r); if (_idx >= 0 && _idx < 25) { cRe[_idx] = _c24r; cIm[_idx] = _c24i; } }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_811_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _mul1r = 0, _mul1i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul1r, &_mul1i);
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul1r; cIm[_idx] = _mul1i; } }
    double _add2r = 0, _add2i = 0;
    _add2r = x1r + x2r; _add2i = x1i + x2i;
    double _sub3r = 0, _sub3i = 0;
    _sub3r = x1r - x2r; _sub3i = x1i - x2i;
    double _conj4r = 0, _conj4i = 0;
    _conj4r = _sub3r; _conj4i = -(_sub3i);
    double _mul5r = 0, _mul5i = 0;
    c_mul(_add2r, _add2i, _conj4r, _conj4i, &_mul5r, &_mul5i);
    { int _idx = 1; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul5r; cIm[_idx] = _mul5i; } }
    for (int k = 2; k < 26; k++) {
        double _abs6r = 0, _abs6i = 0;
        _abs6r = c_abs(x1r, x1i); _abs6i = 0;
        double _abs7r = 0, _abs7i = 0;
        _abs7r = c_abs(x2r, x2i); _abs7i = 0;
        double _mul8r = 0, _mul8i = 0;
        c_mul(_abs6r, _abs6i, _abs7r, _abs7i, &_mul8r, &_mul8i);
        double _c9r = 0, _c9i = 0;
        _c9r = 0.0; _c9i = 1.0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(_c9r, _c9i, x2r, x2i, &_mul10r, &_mul10i);
        double _add11r = 0, _add11i = 0;
        _add11r = x1r + _mul10r; _add11i = x1i + _mul10i;
        double _ang12r = 0, _ang12i = 0;
        _ang12r = c_arg(_add11r, _add11i); _ang12i = 0;
        double _pow13r = 0, _pow13i = 0;
        c_powr(_ang12r, _ang12i, k, &_pow13r, &_pow13i);
        double _sin14r = 0, _sin14i = 0;
        c_sin(_pow13r, _pow13i, &_sin14r, &_sin14i);
        double _mul15r = 0, _mul15i = 0;
        c_mul(_mul8r, _mul8i, _sin14r, _sin14i, &_mul15r, &_mul15i);
        double _pow16r = 0, _pow16i = 0;
        c_powr(x1r, x1i, k, &_pow16r, &_pow16i);
        double _c17r = 0, _c17i = 0;
        _c17r = 1.0; _c17i = 0;
        double _add18r = 0, _add18i = 0;
        _add18r = _c17r + x2r; _add18i = _c17i + x2i;
        double _div19r = 0, _div19i = 0;
        c_div(_pow16r, _pow16i, _add18r, _add18i, &_div19r, &_div19i);
        double _abs20r = 0, _abs20i = 0;
        _abs20r = c_abs(_div19r, _div19i); _abs20i = 0;
        double _log21r = 0, _log21i = 0;
        c_log(_abs20r, _abs20i, &_log21r, &_log21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _mul15r + _log21r; _add22i = _mul15i + _log21i;
        { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add22r; cIm[_idx] = _add22i; } }
    }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_812_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _mul1r = 0, _mul1i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul1r, &_mul1i);
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul1r; cIm[_idx] = _mul1i; } }
    for (int k = 1; k < 26; k++) {
        double _cf2r = 0, _cf2i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 25) { _cf2r = cRe[_idx]; _cf2i = cIm[_idx]; } }
        double _c3r = 0, _c3i = 0;
        _c3r = 2.0; _c3i = 0;
        double _pow4r = 0, _pow4i = 0;
        c_mul(_cf2r, _cf2i, _cf2r, _cf2i, &_pow4r, &_pow4i);
        double _re5r = 0, _re5i = 0;
        _re5r = x1r; _re5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = _pow4r + _re5r; _add6i = _pow4i + _re5i;
        double _c7r = 0, _c7i = 0;
        _c7r = 0.0; _c7i = 1.0;
        double _mul8r = 0, _mul8i = 0;
        c_mul(_c7r, _c7i, x2r, x2i, &_mul8r, &_mul8i);
        double _im9r = 0, _im9i = 0;
        _im9r = _mul8i; _im9i = 0;
        double _add10r = 0, _add10i = 0;
        _add10r = _add6r + _im9r; _add10i = _add6i + _im9i;
        double _c11r = 0, _c11i = 0;
        _c11r = 1.0; _c11i = 0;
        double _cf12r = 0, _cf12i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 25) { _cf12r = cRe[_idx]; _cf12i = cIm[_idx]; } }
        double _abs13r = 0, _abs13i = 0;
        _abs13r = c_abs(_cf12r, _cf12i); _abs13i = 0;
        double _add14r = 0, _add14i = 0;
        _add14r = _c11r + _abs13r; _add14i = _c11i + _abs13i;
        double _div15r = 0, _div15i = 0;
        c_div(_add10r, _add10i, _add14r, _add14i, &_div15r, &_div15i);
        { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _div15r; cIm[_idx] = _div15i; } }
        double _cf16r = 0, _cf16i = 0;
        { int _idx = k; if (_idx >= 0 && _idx < 25) { _cf16r = cRe[_idx]; _cf16i = cIm[_idx]; } }
        double _abs17r = 0, _abs17i = 0;
        _abs17r = c_abs(_cf16r, _cf16i); _abs17i = 0;
        double _c18r = 0, _c18i = 0;
        _c18r = 1000000.0; _c18i = 0;
        double _np19r = 0, _np19i = 0;
        /* WARNING: unhandled np.isnan */
        double _np20r = 0, _np20i = 0;
        /* WARNING: unhandled np.isinf */
        if ((_abs17r > _c18r) || ((_np19r != 0 || _np19i != 0)) || ((_np20r != 0 || _np20i != 0))) {
            double _cf21r = 0, _cf21i = 0;
            { int _idx = (k - 1); if (_idx >= 0 && _idx < 25) { _cf21r = cRe[_idx]; _cf21i = cIm[_idx]; } }
            { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _cf21r; cIm[_idx] = _cf21i; } }
        }
    }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_813_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _mul1r = 0, _mul1i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul1r, &_mul1i);
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul1r; cIm[_idx] = _mul1i; } }
    for (int k = 1; k < 26; k++) {
        double _cf2r = 0, _cf2i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 25) { _cf2r = cRe[_idx]; _cf2i = cIm[_idx]; } }
        double _mul3r = 0, _mul3i = 0;
        c_mul(k, 0, _cf2r, _cf2i, &_mul3r, &_mul3i);
        double _sin4r = 0, _sin4i = 0;
        c_sin(_mul3r, _mul3i, &_sin4r, &_sin4i);
        double _cf5r = 0, _cf5i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 25) { _cf5r = cRe[_idx]; _cf5i = cIm[_idx]; } }
        double _mul6r = 0, _mul6i = 0;
        c_mul(k, 0, _cf5r, _cf5i, &_mul6r, &_mul6i);
        double _cos7r = 0, _cos7i = 0;
        c_cos(_mul6r, _mul6i, &_cos7r, &_cos7i);
        double _add8r = 0, _add8i = 0;
        _add8r = _sin4r + _cos7r; _add8i = _sin4i + _cos7i;
        double _add9r = 0, _add9i = 0;
        _add9r = x1r + x2r; _add9i = x1i + x2i;
        double _re10r = 0, _re10i = 0;
        _re10r = _add9r; _re10i = 0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(_add8r, _add8i, _re10r, _re10i, &_mul11r, &_mul11i);
        double v = _mul11r; /* +_mul11ii */
        double _abs12r = 0, _abs12i = 0;
        _abs12r = c_abs(v, 0); _abs12i = 0;
        double _c13r = 0, _c13i = 0;
        _c13r = 0.0; _c13i = 0;
        if (_abs12r != _c13r) {
            double _abs14r = 0, _abs14i = 0;
            _abs14r = c_abs(v, 0); _abs14i = 0;
            double _log15r = 0, _log15i = 0;
            c_log(_abs14r, _abs14i, &_log15r, &_log15i);
            double _mul16r = 0, _mul16i = 0;
            c_mul(x1r, x1i, x2r, x2i, &_mul16r, &_mul16i);
            double _conj17r = 0, _conj17i = 0;
            _conj17r = _mul16r; _conj17i = -(_mul16i);
            double _add18r = 0, _add18i = 0;
            _add18r = _log15r + _conj17r; _add18i = _log15i + _conj17i;
            { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add18r; cIm[_idx] = _add18i; } }
        } else {
            double _c19r = 0, _c19i = 0;
            _c19r = 0.0; _c19i = 0;
            { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _c19r; cIm[_idx] = _c19i; } }
        }
    }
    double _sum20r = 0, _sum20i = 0;
    _sum20r = 0; _sum20i = 0;
    for (int _si = 0; _si < 24; _si++) { _sum20r += cRe[_si]; _sum20i += cIm[_si]; }
    double _sub21r = 0, _sub21i = 0;
    _sub21r = x1r - x2r; _sub21i = x1i - x2i;
    double _abs22r = 0, _abs22i = 0;
    _abs22r = c_abs(_sub21r, _sub21i); _abs22i = 0;
    double _add23r = 0, _add23i = 0;
    _add23r = _sum20r + _abs22r; _add23i = _sum20i + _abs22i;
    { int _idx = 24; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add23r; cIm[_idx] = _add23i; } }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_814_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _ang1r = 0, _ang1i = 0;
    _ang1r = c_arg(x1r, x1i); _ang1i = 0;
    double _mul2r = 0, _mul2i = 0;
    c_mul(_ang1r, _ang1i, x2r, x2i, &_mul2r, &_mul2i);
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul2r; cIm[_idx] = _mul2i; } }
    double _mul3r = 0, _mul3i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul3r, &_mul3i);
    { int _idx = 1; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul3r; cIm[_idx] = _mul3i; } }
    for (int i = 2; i < 26; i++) {
        double _cf4r = 0, _cf4i = 0;
        { int _idx = (i - 1); if (_idx >= 0 && _idx < 25) { _cf4r = cRe[_idx]; _cf4i = cIm[_idx]; } }
        double _cf5r = 0, _cf5i = 0;
        { int _idx = (i - 2); if (_idx >= 0 && _idx < 25) { _cf5r = cRe[_idx]; _cf5i = cIm[_idx]; } }
        double _add6r = 0, _add6i = 0;
        _add6r = _cf4r + _cf5r; _add6i = _cf4i + _cf5i;
        double _mul7r = 0, _mul7i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul7r, &_mul7i);
        double _conj8r = 0, _conj8i = 0;
        _conj8r = _mul7r; _conj8i = -(_mul7i);
        double _add9r = 0, _add9i = 0;
        _add9r = _add6r + _conj8r; _add9i = _add6i + _conj8i;
        double v = _add9r; /* +_add9ii */
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(v, 0); _abs10i = 0;
        double _c11r = 0, _c11i = 0;
        _c11r = 0.0; _c11i = 0;
        if (_abs10r != _c11r) {
            double _abs12r = 0, _abs12i = 0;
            _abs12r = c_abs(v, 0); _abs12i = 0;
            double _log13r = 0, _log13i = 0;
            c_log(_abs12r, _abs12i, &_log13r, &_log13i);
            { int _idx = i; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _log13r; cIm[_idx] = _log13i; } }
        } else {
            double _c14r = 0, _c14i = 0;
            _c14r = 0.0; _c14i = 0;
            { int _idx = i; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _c14r; cIm[_idx] = _c14i; } }
        }
    }
    double _sub15r = 0, _sub15i = 0;
    _sub15r = x1r - x2r; _sub15i = x1i - x2i;
    double _abs16r = 0, _abs16i = 0;
    _abs16r = c_abs(_sub15r, _sub15i); _abs16i = 0;
    double _sum17r = 0, _sum17i = 0;
    _sum17r = 0; _sum17i = 0;
    for (int _si = 0; _si < 24; _si++) { _sum17r += cRe[_si]; _sum17i += cIm[_si]; }
    double _add18r = 0, _add18i = 0;
    _add18r = _abs16r + _sum17r; _add18i = _abs16i + _sum17i;
    { int _idx = 24; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add18r; cIm[_idx] = _add18i; } }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_815_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 1; k < 26; k++) {
        double _sin1r = 0, _sin1i = 0;
        c_sin(k, 0, &_sin1r, &_sin1i);
        double _mul2r = 0, _mul2i = 0;
        c_mul(_sin1r, _sin1i, x1r, x1i, &_mul2r, &_mul2i);
        double _c3r = 0, _c3i = 0;
        _c3r = 1.0; _c3i = 0;
        double _abs4r = 0, _abs4i = 0;
        _abs4r = c_abs(x2r, x2i); _abs4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = _c3r + _abs4r; _add5i = _c3i + _abs4i;
        double _div6r = 0, _div6i = 0;
        c_div(_mul2r, _mul2i, _add5r, _add5i, &_div6r, &_div6i);
        double _cos7r = 0, _cos7i = 0;
        c_cos(k, 0, &_cos7r, &_cos7i);
        double _mul8r = 0, _mul8i = 0;
        c_mul(_cos7r, _cos7i, x2r, x2i, &_mul8r, &_mul8i);
        double _c9r = 0, _c9i = 0;
        _c9r = 1.0; _c9i = 0;
        double _abs10r = 0, _abs10i = 0;
        _abs10r = c_abs(x1r, x1i); _abs10i = 0;
        double _add11r = 0, _add11i = 0;
        _add11r = _c9r + _abs10r; _add11i = _c9i + _abs10i;
        double _div12r = 0, _div12i = 0;
        c_div(_mul8r, _mul8i, _add11r, _add11i, &_div12r, &_div12i);
        double _add13r = 0, _add13i = 0;
        _add13r = _div6r + _div12r; _add13i = _div6i + _div12i;
        double _sqrt14r = 0, _sqrt14i = 0;
        c_powr(k, 0, 0.5, &_sqrt14r, &_sqrt14i);
        double _add15r = 0, _add15i = 0;
        _add15r = _add13r + _sqrt14r; _add15i = _add13i + _sqrt14i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add15r; cIm[_idx] = _add15i; } }
    }
    double _abs16r = 0, _abs16i = 0;
    _abs16r = c_abs(x1r, x1i); _abs16i = 0;
    double _abs17r = 0, _abs17i = 0;
    _abs17r = c_abs(x2r, x2i); _abs17i = 0;
    double _mul18r = 0, _mul18i = 0;
    c_mul(_abs16r, _abs16i, _abs17r, _abs17i, &_mul18r, &_mul18i);
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul18r; cIm[_idx] = _mul18i; } }
    double _ang19r = 0, _ang19i = 0;
    _ang19r = c_arg(x1r, x1i); _ang19i = 0;
    double _abs20r = 0, _abs20i = 0;
    _abs20r = c_abs(x2r, x2i); _abs20i = 0;
    double _mul21r = 0, _mul21i = 0;
    c_mul(_ang19r, _ang19i, _abs20r, _abs20i, &_mul21r, &_mul21i);
    { int _idx = 4; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul21r; cIm[_idx] = _mul21i; } }
    double _abs22r = 0, _abs22i = 0;
    _abs22r = c_abs(x1r, x1i); _abs22i = 0;
    double _ang23r = 0, _ang23i = 0;
    _ang23r = c_arg(x2r, x2i); _ang23i = 0;
    double _mul24r = 0, _mul24i = 0;
    c_mul(_abs22r, _abs22i, _ang23r, _ang23i, &_mul24r, &_mul24i);
    { int _idx = 9; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul24r; cIm[_idx] = _mul24i; } }
    double _abs25r = 0, _abs25i = 0;
    _abs25r = c_abs(x1r, x1i); _abs25i = 0;
    double _attr26r = 0, _attr26i = 0;
    _attr26r = x2r; _attr26i = 0;
    double _mul27r = 0, _mul27i = 0;
    c_mul(_abs25r, _abs25i, _attr26r, _attr26i, &_mul27r, &_mul27i);
    { int _idx = 14; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul27r; cIm[_idx] = _mul27i; } }
    double _abs28r = 0, _abs28i = 0;
    _abs28r = c_abs(x1r, x1i); _abs28i = 0;
    double _attr29r = 0, _attr29i = 0;
    _attr29r = x2i; _attr29i = 0;
    double _mul30r = 0, _mul30i = 0;
    c_mul(_abs28r, _abs28i, _attr29r, _attr29i, &_mul30r, &_mul30i);
    { int _idx = 19; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul30r; cIm[_idx] = _mul30i; } }
    double _attr31r = 0, _attr31i = 0;
    _attr31r = x1r; _attr31i = 0;
    double _abs32r = 0, _abs32i = 0;
    _abs32r = c_abs(x2r, x2i); _abs32i = 0;
    double _mul33r = 0, _mul33i = 0;
    c_mul(_attr31r, _attr31i, _abs32r, _abs32i, &_mul33r, &_mul33i);
    { int _idx = 24; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul33r; cIm[_idx] = _mul33i; } }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_816_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _mul1r = 0, _mul1i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul1r, &_mul1i);
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul1r; cIm[_idx] = _mul1i; } }
    double _abs2r = 0, _abs2i = 0;
    _abs2r = c_abs(x1r, x1i); _abs2i = 0;
    double _c3r = 0, _c3i = 0;
    _c3r = 2.0; _c3i = 0;
    double _pow4r = 0, _pow4i = 0;
    c_mul(_abs2r, _abs2i, _abs2r, _abs2i, &_pow4r, &_pow4i);
    double _abs5r = 0, _abs5i = 0;
    _abs5r = c_abs(x2r, x2i); _abs5i = 0;
    double _c6r = 0, _c6i = 0;
    _c6r = 2.0; _c6i = 0;
    double _pow7r = 0, _pow7i = 0;
    c_mul(_abs5r, _abs5i, _abs5r, _abs5i, &_pow7r, &_pow7i);
    double _add8r = 0, _add8i = 0;
    _add8r = _pow4r + _pow7r; _add8i = _pow4i + _pow7i;
    { int _idx = 1; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add8r; cIm[_idx] = _add8i; } }
    for (int k = 2; k < 24; k++) {
        double _cf9r = 0, _cf9i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 25) { _cf9r = cRe[_idx]; _cf9i = cIm[_idx]; } }
        double _cf10r = 0, _cf10i = 0;
        { int _idx = (k - 2); if (_idx >= 0 && _idx < 25) { _cf10r = cRe[_idx]; _cf10i = cIm[_idx]; } }
        double _div11r = 0, _div11i = 0;
        c_div(_cf9r, _cf9i, _cf10r, _cf10i, &_div11r, &_div11i);
        double _conj12r = 0, _conj12i = 0;
        _conj12r = x1r; _conj12i = -(x1i);
        double _add13r = 0, _add13i = 0;
        _add13r = _div11r + _conj12r; _add13i = _div11i + _conj12i;
        double _sub14r = 0, _sub14i = 0;
        _sub14r = _add13r - x2r; _sub14i = _add13i - x2i;
        { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _sub14r; cIm[_idx] = _sub14i; } }
        double _np15r = 0, _np15i = 0;
        /* WARNING: unhandled np.isnan */
        double _np16r = 0, _np16i = 0;
        /* WARNING: unhandled np.isinf */
        if (((_np15r != 0 || _np15i != 0)) || ((_np16r != 0 || _np16i != 0))) {
            double _c17r = 0, _c17i = 0;
            _c17r = 0.0; _c17i = 0;
            { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _c17r; cIm[_idx] = _c17i; } }
        }
    }
    double _cf18r = 0, _cf18i = 0;
    { int _idx = 23; if (_idx >= 0 && _idx < 25) { _cf18r = cRe[_idx]; _cf18i = cIm[_idx]; } }
    double _cf19r = 0, _cf19i = 0;
    { int _idx = 22; if (_idx >= 0 && _idx < 25) { _cf19r = cRe[_idx]; _cf19i = cIm[_idx]; } }
    double _add20r = 0, _add20i = 0;
    _add20r = _cf18r + _cf19r; _add20i = _cf18i + _cf19i;
    double _add21r = 0, _add21i = 0;
    _add21r = x1r + x2r; _add21i = x1i + x2i;
    double _conj22r = 0, _conj22i = 0;
    _conj22r = _add21r; _conj22i = -(_add21i);
    double _sub23r = 0, _sub23i = 0;
    _sub23r = _add20r - _conj22r; _sub23i = _add20i - _conj22i;
    { int _idx = 24; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _sub23r; cIm[_idx] = _sub23i; } }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_817_c(double x1r, double x1i, double x2r, double x2i,
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
        _c15r = 1.0; _c15i = 0;
        double _add16r = 0, _add16i = 0;
        _add16r = k + _c15r; _add16i = 0 + _c15i;
        double _c17r = 0, _c17i = 0;
        _c17r = 3.0; _c17i = 0;
        double _mod18r = 0, _mod18i = 0;
        _mod18r = fmod(_add16r, _c17r); _mod18i = 0;
        double _c19r = 0, _c19i = 0;
        _c19r = 0.0; _c19i = 0;
        if (_mod18r == _c19r) {
            double _c20r = 0, _c20i = 0;
            _c20r = 0.0; _c20i = 1.0;
            double _mul21r = 0, _mul21i = 0;
            c_mul(_c20r, _c20i, x2r, x2i, &_mul21r, &_mul21i);
            double _add22r = 0, _add22i = 0;
            _add22r = x1r + _mul21r; _add22i = x1i + _mul21i;
            double _c23r = 0, _c23i = 0;
            _c23r = 1.0; _c23i = 0;
            double _add24r = 0, _add24i = 0;
            _add24r = k + _c23r; _add24i = 0 + _c23i;
            double _c25r = 0, _c25i = 0;
            _c25r = 3.0; _c25i = 0;
            double _div26r = 0, _div26i = 0;
            c_div(_add24r, _add24i, _c25r, _c25i, &_div26r, &_div26i);
            double _pow27r = 0, _pow27i = 0;
            c_powr(_add22r, _add22i, _div26r, &_pow27r, &_pow27i);
            double _c28r = 0, _c28i = 0;
            _c28r = 1.0; _c28i = 0;
            double _add29r = 0, _add29i = 0;
            _add29r = k + _c28r; _add29i = 0 + _c28i;
            double _div30r = 0, _div30i = 0;
            c_div(_pow27r, _pow27i, _add29r, _add29i, &_div30r, &_div30i);
            { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _div30r; cIm[_idx] = _div30i; } }
        } else {
            double _cf31r = 0, _cf31i = 0;
            { int _idx = (k - 1); if (_idx >= 0 && _idx < 25) { _cf31r = cRe[_idx]; _cf31i = cIm[_idx]; } }
            double _conj32r = 0, _conj32i = 0;
            _conj32r = _cf31r; _conj32i = -(_cf31i);
            double _c33r = 0, _c33i = 0;
            _c33r = 2.0; _c33i = 0;
            double _pow34r = 0, _pow34i = 0;
            c_mul(_conj32r, _conj32i, _conj32r, _conj32i, &_pow34r, &_pow34i);
            double _abs35r = 0, _abs35i = 0;
            _abs35r = c_abs(x1r, x1i); _abs35i = 0;
            double _abs36r = 0, _abs36i = 0;
            _abs36r = c_abs(x2r, x2i); _abs36i = 0;
            double _mul37r = 0, _mul37i = 0;
            c_mul(_abs35r, _abs35i, _abs36r, _abs36i, &_mul37r, &_mul37i);
            double _add38r = 0, _add38i = 0;
            _add38r = _pow34r + _mul37r; _add38i = _pow34i + _mul37i;
            { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add38r; cIm[_idx] = _add38i; } }
        }
    }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_818_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int k = 1; k < 26; k++) {
        double _c1r = 0, _c1i = 0;
        _c1r = 0.0; _c1i = 1.0;
        double _mul2r = 0, _mul2i = 0;
        c_mul(_c1r, _c1i, x2r, x2i, &_mul2r, &_mul2i);
        double _add3r = 0, _add3i = 0;
        _add3r = x1r + _mul2r; _add3i = x1i + _mul2i;
        double _c4r = 0, _c4i = 0;
        _c4r = 0.5; _c4i = 0;
        double _add5r = 0, _add5i = 0;
        _add5r = _add3r + _c4r; _add5i = _add3i + _c4i;
        double _c6r = 0, _c6i = 0;
        _c6r = 25.0; _c6i = 0;
        double _div7r = 0, _div7i = 0;
        c_div(k, 0, _c6r, _c6i, &_div7r, &_div7i);
        double _add8r = 0, _add8i = 0;
        _add8r = _add5r + _div7r; _add8i = _add5i + _div7i;
        double _abs9r = 0, _abs9i = 0;
        _abs9r = c_abs(_add8r, _add8i); _abs9i = 0;
        double _c10r = 0, _c10i = 0;
        _c10r = 0.0; _c10i = 1.0;
        double _mul11r = 0, _mul11i = 0;
        c_mul(_c10r, _c10i, x2r, x2i, &_mul11r, &_mul11i);
        double _add12r = 0, _add12i = 0;
        _add12r = x1r + _mul11r; _add12i = x1i + _mul11i;
        double _ang13r = 0, _ang13i = 0;
        _ang13r = c_arg(_add12r, _add12i); _ang13i = 0;
        double _c14r = 0, _c14i = 0;
        _c14r = 1.0; _c14i = 0;
        double _sub15r = 0, _sub15i = 0;
        _sub15r = k - _c14r; _sub15i = 0 - _c14i;
        double _pow16r = 0, _pow16i = 0;
        c_powr(_ang13r, _ang13i, _sub15r, &_pow16r, &_pow16i);
        double _cos17r = 0, _cos17i = 0;
        c_cos(_pow16r, _pow16i, &_cos17r, &_cos17i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_abs9r, _abs9i, _cos17r, _cos17i, &_mul18r, &_mul18i);
        double _c19r = 0, _c19i = 0;
        _c19r = 0.0; _c19i = 1.0;
        double _c20r = 0, _c20i = 0;
        _c20r = 0.0; _c20i = 1.0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(_c20r, _c20i, x1r, x1i, &_mul21r, &_mul21i);
        double _add22r = 0, _add22i = 0;
        _add22r = x2r + _mul21r; _add22i = x2i + _mul21i;
        double _c23r = 0, _c23i = 0;
        _c23r = 0.5; _c23i = 0;
        double _add24r = 0, _add24i = 0;
        _add24r = _add22r + _c23r; _add24i = _add22i + _c23i;
        double _c25r = 0, _c25i = 0;
        _c25r = 25.0; _c25i = 0;
        double _div26r = 0, _div26i = 0;
        c_div(k, 0, _c25r, _c25i, &_div26r, &_div26i);
        double _add27r = 0, _add27i = 0;
        _add27r = _add24r + _div26r; _add27i = _add24i + _div26i;
        double _abs28r = 0, _abs28i = 0;
        _abs28r = c_abs(_add27r, _add27i); _abs28i = 0;
        double _mul29r = 0, _mul29i = 0;
        c_mul(_c19r, _c19i, _abs28r, _abs28i, &_mul29r, &_mul29i);
        double _c30r = 0, _c30i = 0;
        _c30r = 0.0; _c30i = 1.0;
        double _mul31r = 0, _mul31i = 0;
        c_mul(_c30r, _c30i, x1r, x1i, &_mul31r, &_mul31i);
        double _add32r = 0, _add32i = 0;
        _add32r = x2r + _mul31r; _add32i = x2i + _mul31i;
        double _ang33r = 0, _ang33i = 0;
        _ang33r = c_arg(_add32r, _add32i); _ang33i = 0;
        double _c34r = 0, _c34i = 0;
        _c34r = 1.0; _c34i = 0;
        double _sub35r = 0, _sub35i = 0;
        _sub35r = k - _c34r; _sub35i = 0 - _c34i;
        double _pow36r = 0, _pow36i = 0;
        c_powr(_ang33r, _ang33i, _sub35r, &_pow36r, &_pow36i);
        double _sin37r = 0, _sin37i = 0;
        c_sin(_pow36r, _pow36i, &_sin37r, &_sin37i);
        double _mul38r = 0, _mul38i = 0;
        c_mul(_mul29r, _mul29i, _sin37r, _sin37i, &_mul38r, &_mul38i);
        double _add39r = 0, _add39i = 0;
        _add39r = _mul18r + _mul38r; _add39i = _mul18i + _mul38i;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add39r; cIm[_idx] = _add39i; } }
        double _np40r = 0, _np40i = 0;
        /* WARNING: unhandled np.isnan */
        double _np41r = 0, _np41i = 0;
        /* WARNING: unhandled np.isinf */
        if (((_np40r != 0 || _np40i != 0)) || ((_np41r != 0 || _np41i != 0))) {
            double _c42r = 0, _c42i = 0;
            _c42r = 0.0; _c42i = 0;
            { int _idx = (k - 1); if (_idx >= 0 && _idx < 25) { cRe[_idx] = _c42r; cIm[_idx] = _c42i; } }
        }
    }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_819_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _mul1r = 0, _mul1i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul1r, &_mul1i);
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul1r; cIm[_idx] = _mul1i; } }
    for (int i = 1; i < 25; i++) {
        double _cf2r = 0, _cf2i = 0;
        { int _idx = (i - 1); if (_idx >= 0 && _idx < 25) { _cf2r = cRe[_idx]; _cf2i = cIm[_idx]; } }
        double _attr3r = 0, _attr3i = 0;
        _attr3r = _cf2r; _attr3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 2.0; _c4i = 0;
        double _pow5r = 0, _pow5i = 0;
        c_mul(_attr3r, _attr3i, _attr3r, _attr3i, &_pow5r, &_pow5i);
        double _cf6r = 0, _cf6i = 0;
        { int _idx = (i - 1); if (_idx >= 0 && _idx < 25) { _cf6r = cRe[_idx]; _cf6i = cIm[_idx]; } }
        double _attr7r = 0, _attr7i = 0;
        _attr7r = _cf6i; _attr7i = 0;
        double _c8r = 0, _c8i = 0;
        _c8r = 2.0; _c8i = 0;
        double _pow9r = 0, _pow9i = 0;
        c_mul(_attr7r, _attr7i, _attr7r, _attr7i, &_pow9r, &_pow9i);
        double _sub10r = 0, _sub10i = 0;
        _sub10r = _pow5r - _pow9r; _sub10i = _pow5i - _pow9i;
        double _mul11r = 0, _mul11i = 0;
        c_mul(_sub10r, _sub10i, x1r, x1i, &_mul11r, &_mul11i);
        double _mul12r = 0, _mul12i = 0;
        c_mul(_mul11r, _mul11i, x2r, x2i, &_mul12r, &_mul12i);
        double _c13r = 0, _c13i = 0;
        _c13r = 0.0; _c13i = 1.0;
        double _c14r = 0, _c14i = 0;
        _c14r = 2.0; _c14i = 0;
        double _mul15r = 0, _mul15i = 0;
        c_mul(_c13r, _c13i, _c14r, _c14i, &_mul15r, &_mul15i);
        double _cf16r = 0, _cf16i = 0;
        { int _idx = (i - 1); if (_idx >= 0 && _idx < 25) { _cf16r = cRe[_idx]; _cf16i = cIm[_idx]; } }
        double _attr17r = 0, _attr17i = 0;
        _attr17r = _cf16r; _attr17i = 0;
        double _mul18r = 0, _mul18i = 0;
        c_mul(_mul15r, _mul15i, _attr17r, _attr17i, &_mul18r, &_mul18i);
        double _cf19r = 0, _cf19i = 0;
        { int _idx = (i - 1); if (_idx >= 0 && _idx < 25) { _cf19r = cRe[_idx]; _cf19i = cIm[_idx]; } }
        double _attr20r = 0, _attr20i = 0;
        _attr20r = _cf19i; _attr20i = 0;
        double _mul21r = 0, _mul21i = 0;
        c_mul(_mul18r, _mul18i, _attr20r, _attr20i, &_mul21r, &_mul21i);
        double _add22r = 0, _add22i = 0;
        _add22r = _mul12r + _mul21r; _add22i = _mul12i + _mul21i;
        { int _idx = i; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add22r; cIm[_idx] = _add22i; } }
        double _np23r = 0, _np23i = 0;
        /* WARNING: unhandled np.isnan */
        if ((_np23r != 0 || _np23i != 0)) {
            double _c24r = 0, _c24i = 0;
            _c24r = 1.0; _c24i = 0;
            { int _idx = i; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _c24r; cIm[_idx] = _c24i; } }
        }
        double _cf25r = 0, _cf25i = 0;
        { int _idx = i; if (_idx >= 0 && _idx < 25) { _cf25r = cRe[_idx]; _cf25i = cIm[_idx]; } }
        double _abs26r = 0, _abs26i = 0;
        _abs26r = c_abs(_cf25r, _cf25i); _abs26i = 0;
        double _c27r = 0, _c27i = 0;
        _c27r = 1e-10; _c27i = 0;
        if (_abs26r < _c27r) {
            double _c28r = 0, _c28i = 0;
            _c28r = 1.0; _c28i = 0;
            { int _idx = i; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _c28r; cIm[_idx] = _c28i; } }
        }
    }
    double _cf29r = 0, _cf29i = 0;
    { int _idx = 23; if (_idx >= 0 && _idx < 25) { _cf29r = cRe[_idx]; _cf29i = cIm[_idx]; } }
    double _add30r = 0, _add30i = 0;
    _add30r = _cf29r + x1r; _add30i = _cf29i + x1i;
    double _add31r = 0, _add31i = 0;
    _add31r = _add30r + x2r; _add31i = _add30i + x2i;
    { int _idx = 24; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add31r; cIm[_idx] = _add31i; } }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_820_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _add1r = 0, _add1i = 0;
    _add1r = x1r + x2r; _add1i = x1i + x2i;
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add1r; cIm[_idx] = _add1i; } }
    double _c2r = 0, _c2i = 0;
    _c2r = 5.0; _c2i = 0;
    double _mul3r = 0, _mul3i = 0;
    c_mul(x1r, x1i, x2r, x2i, &_mul3r, &_mul3i);
    double _re4r = 0, _re4i = 0;
    _re4r = _mul3r; _re4i = 0;
    double _mul5r = 0, _mul5i = 0;
    c_mul(_c2r, _c2i, _re4r, _re4i, &_mul5r, &_mul5i);
    { int _idx = 4; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul5r; cIm[_idx] = _mul5i; } }
    double _c6r = 0, _c6i = 0;
    _c6r = 9.0; _c6i = 0;
    double _conj7r = 0, _conj7i = 0;
    _conj7r = x2r; _conj7i = -(x2i);
    double _mul8r = 0, _mul8i = 0;
    c_mul(x1r, x1i, _conj7r, _conj7i, &_mul8r, &_mul8i);
    double _ang9r = 0, _ang9i = 0;
    _ang9r = c_arg(_mul8r, _mul8i); _ang9i = 0;
    double _sin10r = 0, _sin10i = 0;
    c_sin(_ang9r, _ang9i, &_sin10r, &_sin10i);
    double _mul11r = 0, _mul11i = 0;
    c_mul(_c6r, _c6i, _sin10r, _sin10i, &_mul11r, &_mul11i);
    { int _idx = 8; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul11r; cIm[_idx] = _mul11i; } }
    for (int i = 1; i < 4; i++) {
        double _cf12r = 0, _cf12i = 0;
        { int _idx = (i - 1); if (_idx >= 0 && _idx < 25) { _cf12r = cRe[_idx]; _cf12i = cIm[_idx]; } }
        double _c13r = 0, _c13i = 0;
        _c13r = 2.0; _c13i = 0;
        double _pow14r = 0, _pow14i = 0;
        c_mul(_cf12r, _cf12i, _cf12r, _cf12i, &_pow14r, &_pow14i);
        double _cf15r = 0, _cf15i = 0;
        { int _idx = 4; if (_idx >= 0 && _idx < 25) { _cf15r = cRe[_idx]; _cf15i = cIm[_idx]; } }
        double _add16r = 0, _add16i = 0;
        _add16r = _pow14r + _cf15r; _add16i = _pow14i + _cf15i;
        { int _idx = i; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add16r; cIm[_idx] = _add16i; } }
    }
    for (int j = 5; j < 8; j++) {
        double _abs17r = 0, _abs17i = 0;
        _abs17r = c_abs(x1r, x1i); _abs17i = 0;
        double _cf18r = 0, _cf18i = 0;
        { int _idx = (j - 1); if (_idx >= 0 && _idx < 25) { _cf18r = cRe[_idx]; _cf18i = cIm[_idx]; } }
        double _pow19r = 0, _pow19i = 0;
        c_powr(_abs17r, _abs17i, _cf18r, &_pow19r, &_pow19i);
        double _cf20r = 0, _cf20i = 0;
        { int _idx = 0; if (_idx >= 0 && _idx < 25) { _cf20r = cRe[_idx]; _cf20i = cIm[_idx]; } }
        double _add21r = 0, _add21i = 0;
        _add21r = _pow19r + _cf20r; _add21i = _pow19i + _cf20i;
        { int _idx = j; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add21r; cIm[_idx] = _add21i; } }
    }
    for (int k = 9; k < 25; k++) {
        double _cf22r = 0, _cf22i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 25) { _cf22r = cRe[_idx]; _cf22i = cIm[_idx]; } }
        double _abs23r = 0, _abs23i = 0;
        _abs23r = c_abs(_cf22r, _cf22i); _abs23i = 0;
        double _c24r = 0, _c24i = 0;
        _c24r = 1.0; _c24i = 0;
        double _add25r = 0, _add25i = 0;
        _add25r = _abs23r + _c24r; _add25i = _abs23i + _c24i;
        double _log26r = 0, _log26i = 0;
        c_log(_add25r, _add25i, &_log26r, &_log26i);
        double _cf27r = 0, _cf27i = 0;
        { int _idx = 8; if (_idx >= 0 && _idx < 25) { _cf27r = cRe[_idx]; _cf27i = cIm[_idx]; } }
        double _add28r = 0, _add28i = 0;
        _add28r = _log26r + _cf27r; _add28i = _log26i + _cf27i;
        { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add28r; cIm[_idx] = _add28i; } }
    }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_821_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 25;
    for (int _i = 0; _i < 25; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 3.0; _c1i = 0;
    double _mul2r = 0, _mul2i = 0;
    c_mul(_c1r, _c1i, x1r, x1i, &_mul2r, &_mul2i);
    double _c3r = 0, _c3i = 0;
    _c3r = 0.0; _c3i = 5.0;
    double _mul4r = 0, _mul4i = 0;
    c_mul(_c3r, _c3i, x2r, x2i, &_mul4r, &_mul4i);
    double _add5r = 0, _add5i = 0;
    _add5r = _mul2r + _mul4r; _add5i = _mul2i + _mul4i;
    { int _idx = 0; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _add5r; cIm[_idx] = _add5i; } }
    for (int k = 1; k < 25; k++) {
        double _abs6r = 0, _abs6i = 0;
        _abs6r = c_abs(x1r, x1i); _abs6i = 0;
        double mod_t1 = _abs6r; /* +_abs6ii */
        double _ang7r = 0, _ang7i = 0;
        _ang7r = c_arg(x2r, x2i); _ang7i = 0;
        double arg_t2 = _ang7r; /* +_ang7ii */
        double _cf8r = 0, _cf8i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 25) { _cf8r = cRe[_idx]; _cf8i = cIm[_idx]; } }
        double _add9r = 0, _add9i = 0;
        _add9r = mod_t1 + arg_t2; _add9i = 0 + 0;
        double _mul10r = 0, _mul10i = 0;
        c_mul(_cf8r, _cf8i, _add9r, _add9i, &_mul10r, &_mul10i);
        { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _mul10r; cIm[_idx] = _mul10i; } }
        double _cf11r = 0, _cf11i = 0;
        { int _idx = k; if (_idx >= 0 && _idx < 25) { _cf11r = cRe[_idx]; _cf11i = cIm[_idx]; } }
        double _attr12r = 0, _attr12i = 0;
        _attr12r = _cf11r; _attr12i = 0;
        double _c13r = 0, _c13i = 0;
        _c13r = 0.0; _c13i = 0;
        double _cf14r = 0, _cf14i = 0;
        { int _idx = k; if (_idx >= 0 && _idx < 25) { _cf14r = cRe[_idx]; _cf14i = cIm[_idx]; } }
        double _attr15r = 0, _attr15i = 0;
        _attr15r = _cf14i; _attr15i = 0;
        double _c16r = 0, _c16i = 0;
        _c16r = 0.0; _c16i = 0;
        if ((_attr12r < _c13r) && (_attr15r < _c16r)) {
            double _cf17r = 0, _cf17i = 0;
            { int _idx = k; if (_idx >= 0 && _idx < 25) { _cf17r = cRe[_idx]; _cf17i = cIm[_idx]; } }
            double _conj18r = 0, _conj18i = 0;
            _conj18r = _cf17r; _conj18i = -(_cf17i);
            { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _conj18r; cIm[_idx] = _conj18i; } }
        }
        double _cf19r = 0, _cf19i = 0;
        { int _idx = k; if (_idx >= 0 && _idx < 25) { _cf19r = cRe[_idx]; _cf19i = cIm[_idx]; } }
        double _attr20r = 0, _attr20i = 0;
        _attr20r = _cf19r; _attr20i = 0;
        double _abs21r = 0, _abs21i = 0;
        _abs21r = c_abs(_attr20r, _attr20i); _abs21i = 0;
        double _c22r = 0, _c22i = 0;
        _c22r = 10.0; _c22i = 0;
        if (_abs21r > _c22r) {
            double _cf23r = 0, _cf23i = 0;
            { int _idx = k; if (_idx >= 0 && _idx < 25) { _cf23r = cRe[_idx]; _cf23i = cIm[_idx]; } }
            double _div24r = 0, _div24i = 0;
            c_div(_cf23r, _cf23i, mod_t1, 0, &_div24r, &_div24i);
            { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _div24r; cIm[_idx] = _div24i; } }
        }
        double _cf25r = 0, _cf25i = 0;
        { int _idx = k; if (_idx >= 0 && _idx < 25) { _cf25r = cRe[_idx]; _cf25i = cIm[_idx]; } }
        double _attr26r = 0, _attr26i = 0;
        _attr26r = _cf25i; _attr26i = 0;
        double _abs27r = 0, _abs27i = 0;
        _abs27r = c_abs(_attr26r, _attr26i); _abs27i = 0;
        double _c28r = 0, _c28i = 0;
        _c28r = 10.0; _c28i = 0;
        if (_abs27r > _c28r) {
            double _cf29r = 0, _cf29i = 0;
            { int _idx = k; if (_idx >= 0 && _idx < 25) { _cf29r = cRe[_idx]; _cf29i = cIm[_idx]; } }
            double _c30r = 0, _c30i = 0;
            _c30r = 0.0; _c30i = 1.0;
            double _mul31r = 0, _mul31i = 0;
            c_mul(_c30r, _c30i, arg_t2, 0, &_mul31r, &_mul31i);
            double _div32r = 0, _div32i = 0;
            c_div(_cf29r, _cf29i, _mul31r, _mul31i, &_div32r, &_div32i);
            { int _idx = k; if (_idx >= 0 && _idx < 25) { cRe[_idx] = _div32r; cIm[_idx] = _div32i; } }
        }
    }
    for (int _i = 0; _i < 25; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_creative3: too complex for auto-transpile, stubbed */
static void poly_creative3_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_creative4_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _unk1r = 0, _unk1i = 0;
    /* WARNING: unhandled node BoolOp(op=Or(), values=[Call(func=Attribute(value=Attribute(value=Name(id='ps', ctx=Load()), attr='poly', ctx=Load()), attr='get', ctx=Load()), args=[Constant(value='n')]), Constant(value=71)]) */
    double n = _unk1r; /* +_unk1ii */
    double _abs2r = 0, _abs2i = 0;
    _abs2r = c_abs(x1r, x1i); _abs2i = 0;
    double _c3r = 0, _c3i = 0;
    _c3r = 100.0; _c3i = 0;
    double _mul4r = 0, _mul4i = 0;
    c_mul(_abs2r, _abs2i, _c3r, _c3i, &_mul4r, &_mul4i);
    double _int5r = 0, _int5i = 0;
    _int5r = (int)(_mul4r); _int5i = 0;
    double _c6r = 0, _c6i = 0;
    _c6r = 13.0; _c6i = 0;
    double _mod7r = 0, _mod7i = 0;
    _mod7r = fmod(_int5r, _c6r); _mod7i = 0;
    double _c8r = 0, _c8i = 0;
    _c8r = 1.0; _c8i = 0;
    double _add9r = 0, _add9i = 0;
    _add9r = _mod7r + _c8r; _add9i = _mod7i + _c8i;
    double mod1 = _add9r; /* +_add9ii */
    double _abs10r = 0, _abs10i = 0;
    _abs10r = c_abs(x2r, x2i); _abs10i = 0;
    double _c11r = 0, _c11i = 0;
    _c11r = 100.0; _c11i = 0;
    double _mul12r = 0, _mul12i = 0;
    c_mul(_abs10r, _abs10i, _c11r, _c11i, &_mul12r, &_mul12i);
    double _int13r = 0, _int13i = 0;
    _int13r = (int)(_mul12r); _int13i = 0;
    double _c14r = 0, _c14i = 0;
    _c14r = 17.0; _c14i = 0;
    double _mod15r = 0, _mod15i = 0;
    _mod15r = fmod(_int13r, _c14r); _mod15i = 0;
    double _c16r = 0, _c16i = 0;
    _c16r = 1.0; _c16i = 0;
    double _add17r = 0, _add17i = 0;
    _add17r = _mod15r + _c16r; _add17i = _mod15i + _c16i;
    double mod2 = _add17r; /* +_add17ii */
    for (int k = 0; k < (int)(n); k++) {
        double _mod18r = 0, _mod18i = 0;
        _mod18r = fmod(k, mod1); _mod18i = 0;
        double _attr19r = 0, _attr19i = 0;
        _attr19r = x1r; _attr19i = 0;
        double _mul20r = 0, _mul20i = 0;
        c_mul(_mod18r, _mod18i, _attr19r, _attr19i, &_mul20r, &_mul20i);
        double _mod21r = 0, _mod21i = 0;
        _mod21r = fmod(k, mod2); _mod21i = 0;
        double _attr22r = 0, _attr22i = 0;
        _attr22r = x2i; _attr22i = 0;
        double _mul23r = 0, _mul23i = 0;
        c_mul(_mod21r, _mod21i, _attr22r, _attr22i, &_mul23r, &_mul23i);
        double _add24r = 0, _add24i = 0;
        _add24r = _mul20r + _mul23r; _add24i = _mul20i + _mul23i;
        double freq = _add24r; /* +_add24ii */
        double _np25r = 0, _np25i = 0;
        /* WARNING: unhandled np.phase */
        double _mul26r = 0, _mul26i = 0;
        c_mul(k, 0, _np25r, _np25i, &_mul26r, &_mul26i);
        double _c27r = 0, _c27i = 0;
        _c27r = 1.0; _c27i = 0;
        double _add28r = 0, _add28i = 0;
        _add28r = k + _c27r; _add28i = 0 + _c27i;
        double _div29r = 0, _div29i = 0;
        c_div(_mul26r, _mul26i, _add28r, _add28i, &_div29r, &_div29i);
        double _np30r = 0, _np30i = 0;
        /* WARNING: unhandled np.phase */
        double _mul31r = 0, _mul31i = 0;
        c_mul(k, 0, _np30r, _np30i, &_mul31r, &_mul31i);
        double _c32r = 0, _c32i = 0;
        _c32r = 1.0; _c32i = 0;
        double _add33r = 0, _add33i = 0;
        _add33r = k + _c32r; _add33i = 0 + _c32i;
        double _div34r = 0, _div34i = 0;
        c_div(_mul31r, _mul31i, _add33r, _add33i, &_div34r, &_div34i);
        double _add35r = 0, _add35i = 0;
        _add35r = _div29r + _div34r; _add35i = _div29i + _div34i;
        double phase = _add35r; /* +_add35ii */
        double _sin36r = 0, _sin36i = 0;
        c_sin(freq, 0, &_sin36r, &_sin36i);
        double _c37r = 0, _c37i = 0;
        _c37r = 0.0; _c37i = 1.0;
        double _cos38r = 0, _cos38i = 0;
        c_cos(freq, 0, &_cos38r, &_cos38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(_c37r, _c37i, _cos38r, _cos38i, &_mul39r, &_mul39i);
        double _add40r = 0, _add40i = 0;
        _add40r = _sin36r + _mul39r; _add40i = _sin36i + _mul39i;
        double _c41r = 0, _c41i = 0;
        _c41r = 0.0; _c41i = 1.0;
        double _mul42r = 0, _mul42i = 0;
        c_mul(_c41r, _c41i, phase, 0, &_mul42r, &_mul42i);
        double _exp43r = 0, _exp43i = 0;
        c_exp2(_mul42r, _mul42i, &_exp43r, &_exp43i);
        double _mul44r = 0, _mul44i = 0;
        c_mul(_add40r, _add40i, _exp43r, _exp43i, &_mul44r, &_mul44i);
        { int _idx = k; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul44r; cIm[_idx] = _mul44i; } }
        double _abs45r = 0, _abs45i = 0;
        _abs45r = c_abs(x1r, x1i); _abs45i = 0;
        double _div46r = 0, _div46i = 0;
        c_div(k, 0, mod1, 0, &_div46r, &_div46i);
        double _pow47r = 0, _pow47i = 0;
        c_powr(_abs45r, _abs45i, _div46r, &_pow47r, &_pow47i);
        double _abs48r = 0, _abs48i = 0;
        _abs48r = c_abs(x2r, x2i); _abs48i = 0;
        double _div49r = 0, _div49i = 0;
        c_div(k, 0, mod2, 0, &_div49r, &_div49i);
        double _pow50r = 0, _pow50i = 0;
        c_powr(_abs48r, _abs48i, _div49r, &_pow50r, &_pow50i);
        double _add51r = 0, _add51i = 0;
        _add51r = _pow47r + _pow50r; _add51i = _pow47i + _pow50i;
        { double _tr = cRe[k]*_add51r - cIm[k]*_add51i; cIm[k] = cRe[k]*_add51i + cIm[k]*_add51r; cRe[k] = _tr; }
    }
    double _attr52r = 0, _attr52i = 0;
    _attr52r = x1i; _attr52i = 0;
    double _c53r = 0, _c53i = 0;
    _c53r = 10.0; _c53i = 0;
    double _mul54r = 0, _mul54i = 0;
    c_mul(_attr52r, _attr52i, _c53r, _c53i, &_mul54r, &_mul54i);
    double _int55r = 0, _int55i = 0;
    _int55r = (int)(_mul54r); _int55i = 0;
    double _mod56r = 0, _mod56i = 0;
    _mod56r = fmod(_int55r, n); _mod56i = 0;
    double shift = _mod56r; /* +_mod56ii */
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_creative5_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _unk1r = 0, _unk1i = 0;
    /* WARNING: unhandled node BoolOp(op=Or(), values=[Call(func=Attribute(value=Attribute(value=Name(id='ps', ctx=Load()), attr='poly', ctx=Load()), attr='get', ctx=Load()), args=[Constant(value='n')]), Constant(value=71)]) */
    double n = _unk1r; /* +_unk1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 0.5; _c2i = 0;
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1r; _attr3i = 0;
    double _attr4r = 0, _attr4i = 0;
    _attr4r = x1i; _attr4i = 0;
    double _add5r = 0, _add5i = 0;
    _add5r = _attr3r + _attr4r; _add5i = _attr3i + _attr4i;
    double _mul6r = 0, _mul6i = 0;
    c_mul(_c2r, _c2i, _add5r, _add5i, &_mul6r, &_mul6i);
    double _abs7r = 0, _abs7i = 0;
    _abs7r = c_abs(x1r, x1i); _abs7i = 0;
    double _c8r = 0, _c8i = 0;
    _c8r = 1e-06; _c8i = 0;
    double _add9r = 0, _add9i = 0;
    _add9r = _abs7r + _c8r; _add9i = _abs7i + _c8i;
    double _div10r = 0, _div10i = 0;
    c_div(_mul6r, _mul6i, _add9r, _add9i, &_div10r, &_div10i);
    double x = _div10r; /* +_div10ii */
    double _c11r = 0, _c11i = 0;
    _c11r = 3.7; _c11i = 0;
    double _c12r = 0, _c12i = 0;
    _c12r = 0.3; _c12i = 0;
    double _attr13r = 0, _attr13i = 0;
    _attr13r = x2r; _attr13i = 0;
    double _attr14r = 0, _attr14i = 0;
    _attr14r = x2i; _attr14i = 0;
    double _sub15r = 0, _sub15i = 0;
    _sub15r = _attr13r - _attr14r; _sub15i = _attr13i - _attr14i;
    double _mul16r = 0, _mul16i = 0;
    c_mul(_c12r, _c12i, _sub15r, _sub15i, &_mul16r, &_mul16i);
    double _abs17r = 0, _abs17i = 0;
    _abs17r = c_abs(x2r, x2i); _abs17i = 0;
    double _c18r = 0, _c18i = 0;
    _c18r = 1e-06; _c18i = 0;
    double _add19r = 0, _add19i = 0;
    _add19r = _abs17r + _c18r; _add19i = _abs17i + _c18i;
    double _div20r = 0, _div20i = 0;
    c_div(_mul16r, _mul16i, _add19r, _add19i, &_div20r, &_div20i);
    double _add21r = 0, _add21i = 0;
    _add21r = _c11r + _div20r; _add21i = _c11i + _div20i;
    double r = _add21r; /* +_add21ii */
    for (int k = 0; k < (int)(n); k++) {
        double _mul22r = 0, _mul22i = 0;
        c_mul(r, 0, x, 0, &_mul22r, &_mul22i);
        double _c23r = 0, _c23i = 0;
        _c23r = 1.0; _c23i = 0;
        double _sub24r = 0, _sub24i = 0;
        _sub24r = _c23r - x; _sub24i = _c23i - 0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(_mul22r, _mul22i, _sub24r, _sub24i, &_mul25r, &_mul25i);
        x = _mul25r;
        double _c26r = 0, _c26i = 0;
        _c26r = 2.0; _c26i = 0;
        double _mul27r = 0, _mul27i = 0;
        c_mul(_c26r, _c26i, M_PI, 0, &_mul27r, &_mul27i);
        double _mul28r = 0, _mul28i = 0;
        c_mul(_mul27r, _mul27i, x, 0, &_mul28r, &_mul28i);
        double angle = _mul28r; /* +_mul28ii */
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
        c_mul(x, 0, _add33r, _add33i, &_mul34r, &_mul34i);
        { int _idx = k; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul34r; cIm[_idx] = _mul34i; } }
    }
    double _np35r = 0, _np35i = 0;
    /* WARNING: unhandled np.max */
    double _c36r = 0, _c36i = 0;
    _c36r = 1e-06; _c36i = 0;
    double _add37r = 0, _add37i = 0;
    _add37r = _np35r + _c36r; _add37i = _np35i + _c36i;
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_creative6_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _unk1r = 0, _unk1i = 0;
    /* WARNING: unhandled node BoolOp(op=Or(), values=[Call(func=Attribute(value=Attribute(value=Name(id='ps', ctx=Load()), attr='poly', ctx=Load()), attr='get', ctx=Load()), args=[Constant(value='n')]), Constant(value=71)]) */
    double n = _unk1r; /* +_unk1ii */
    for (int k = 0; k < (int)(n); k++) {
        double _c2r = 0, _c2i = 0;
        _c2r = 0.0; _c2i = 1.0;
        double _np3r = 0, _np3i = 0;
        /* WARNING: unhandled np.phase */
        double _mul4r = 0, _mul4i = 0;
        c_mul(k, 0, _np3r, _np3i, &_mul4r, &_mul4i);
        double _abs5r = 0, _abs5i = 0;
        _abs5r = c_abs(x1r, x1i); _abs5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = _mul4r + _abs5r; _add6i = _mul4i + _abs5i;
        double _mul7r = 0, _mul7i = 0;
        c_mul(_c2r, _c2i, _add6r, _add6i, &_mul7r, &_mul7i);
        double _exp8r = 0, _exp8i = 0;
        c_exp2(_mul7r, _mul7i, &_exp8r, &_exp8i);
        double state1 = _exp8r; /* +_exp8ii */
        double _c9r = 0, _c9i = 0;
        _c9r = 0.0; _c9i = 1.0;
        double _neg10r = 0, _neg10i = 0;
        _neg10r = -(_c9r); _neg10i = -(_c9i);
        double _np11r = 0, _np11i = 0;
        /* WARNING: unhandled np.phase */
        double _mul12r = 0, _mul12i = 0;
        c_mul(k, 0, _np11r, _np11i, &_mul12r, &_mul12i);
        double _abs13r = 0, _abs13i = 0;
        _abs13r = c_abs(x2r, x2i); _abs13i = 0;
        double _sub14r = 0, _sub14i = 0;
        _sub14r = _mul12r - _abs13r; _sub14i = _mul12i - _abs13i;
        double _mul15r = 0, _mul15i = 0;
        c_mul(_neg10r, _neg10i, _sub14r, _sub14i, &_mul15r, &_mul15i);
        double _exp16r = 0, _exp16i = 0;
        c_exp2(_mul15r, _mul15i, &_exp16r, &_exp16i);
        double state2 = _exp16r; /* +_exp16ii */
        double _add17r = 0, _add17i = 0;
        _add17r = state1 + state2; _add17i = 0 + 0;
        double _c18r = 0, _c18i = 0;
        _c18r = 2.0; _c18i = 0;
        double _div19r = 0, _div19i = 0;
        c_div(_add17r, _add17i, _c18r, _c18i, &_div19r, &_div19i);
        { int _idx = k; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _div19r; cIm[_idx] = _div19i; } }
        double _c20r = 0, _c20i = 0;
        _c20r = 0.0; _c20i = 0;
        if (k > _c20r) {
            double _cf21r = 0, _cf21i = 0;
            { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { _cf21r = cRe[_idx]; _cf21i = cIm[_idx]; } }
            double _attr22r = 0, _attr22i = 0;
            _attr22r = x1r; _attr22i = 0;
            double _c23r = 0, _c23i = 0;
            _c23r = 0.0; _c23i = 1.0;
            double _attr24r = 0, _attr24i = 0;
            _attr24r = x2i; _attr24i = 0;
            double _mul25r = 0, _mul25i = 0;
            c_mul(_c23r, _c23i, _attr24r, _attr24i, &_mul25r, &_mul25i);
            double _add26r = 0, _add26i = 0;
            _add26r = _attr22r + _mul25r; _add26i = _attr22i + _mul25i;
            double _mul27r = 0, _mul27i = 0;
            c_mul(_cf21r, _cf21i, _add26r, _add26i, &_mul27r, &_mul27i);
            cRe[k] += _mul27r; cIm[k] += _mul27i;
        }
    }
    double _sum28r = 0, _sum28i = 0;
    _sum28r = 0; _sum28i = 0;
    for (int _si = 0; _si < 36; _si++) { _sum28r += cRe[_si]; _sum28i += cIm[_si]; }
    double _c29r = 0, _c29i = 0;
    _c29r = 1.0; _c29i = 0;
    double _add30r = 0, _add30i = 0;
    _add30r = n + _c29r; _add30i = 0 + _c29i;
    double _div31r = 0, _div31i = 0;
    c_div(_sum28r, _sum28i, _add30r, _add30i, &_div31r, &_div31i);
    double global_phase = _div31r; /* +_div31ii */
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_creative7_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _unk1r = 0, _unk1i = 0;
    /* WARNING: unhandled node BoolOp(op=Or(), values=[Call(func=Attribute(value=Attribute(value=Name(id='ps', ctx=Load()), attr='poly', ctx=Load()), attr='get', ctx=Load()), args=[Constant(value='n')]), Constant(value=71)]) */
    double n = _unk1r; /* +_unk1ii */
    for (int k = 0; k < (int)(n); k++) {
        double z = x1r;
        double c = x2r;
        for (int _ = 0; _ < (int)(n); _++) {
            double _c2r = 0, _c2i = 0;
            _c2r = 4.0; _c2i = 0;
            double _pow3r = 0, _pow3i = 0;
            c_mul(z, 0, z, 0, &_pow3r, &_pow3i);
            c_mul(_pow3r, _pow3i, _pow3r, _pow3i, &_pow3r, &_pow3i);
            double _add4r = 0, _add4i = 0;
            _add4r = _pow3r + c; _add4i = _pow3i + 0;
            z = _add4r;
        }
        double _c5r = 0, _c5i = 0;
        _c5r = 1.0; _c5i = 0;
        double _add6r = 0, _add6i = 0;
        _add6r = k + _c5r; _add6i = 0 + _c5i;
        double _div7r = 0, _div7i = 0;
        c_div(z, 0, _add6r, _add6i, &_div7r, &_div7i);
        { int _idx = k; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _div7r; cIm[_idx] = _div7i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_creative8_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _unk1r = 0, _unk1i = 0;
    /* WARNING: unhandled node BoolOp(op=Or(), values=[Call(func=Attribute(value=Attribute(value=Name(id='ps', ctx=Load()), attr='poly', ctx=Load()), attr='get', ctx=Load()), args=[Constant(value='n')]), Constant(value=71)]) */
    double n = _unk1r; /* +_unk1ii */
    for (int k = 0; k < (int)(n); k++) {
        double p = 0;
        double q = 0;
        double _c2r = 0, _c2i = 0;
        _c2r = 2.0; _c2i = 0;
        double _mod3r = 0, _mod3i = 0;
        _mod3r = fmod(k, _c2r); _mod3i = 0;
        double _c4r = 0, _c4i = 0;
        _c4r = 0.0; _c4i = 0;
        if (_mod3r == _c4r) {
            double _c5r = 0, _c5i = 0;
            _c5r = 2.0; _c5i = 0;
            double _fdiv6r = 0, _fdiv6i = 0;
            c_div(k, 0, _c5r, _c5i, &_fdiv6r, &_fdiv6i);
            _fdiv6r = floor(_fdiv6r); _fdiv6i = 0;
            double _c7r = 0, _c7i = 0;
            _c7r = 1.0; _c7i = 0;
            double _add8r = 0, _add8i = 0;
            _add8r = _fdiv6r + _c7r; _add8i = _fdiv6i + _c7i;
            double _attr9r = 0, _attr9i = 0;
            _attr9r = x1r; _attr9i = 0;
            double _mul10r = 0, _mul10i = 0;
            c_mul(_add8r, _add8i, _attr9r, _attr9i, &_mul10r, &_mul10i);
            q = _mul10r;
            double _c11r = 0, _c11i = 0;
            _c11r = 2.0; _c11i = 0;
            double _pow12r = 0, _pow12i = 0;
            c_mul(q, 0, q, 0, &_pow12r, &_pow12i);
            double _c13r = 0, _c13i = 0;
            _c13r = 0.0; _c13i = 1.0;
            double _mul14r = 0, _mul14i = 0;
            c_mul(_c13r, _c13i, q, 0, &_mul14r, &_mul14i);
            double _attr15r = 0, _attr15i = 0;
            _attr15r = x2i; _attr15i = 0;
            double _mul16r = 0, _mul16i = 0;
            c_mul(_mul14r, _mul14i, _attr15r, _attr15i, &_mul16r, &_mul16i);
            double _add17r = 0, _add17i = 0;
            _add17r = _pow12r + _mul16r; _add17i = _pow12i + _mul16i;
            { int _idx = k; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add17r; cIm[_idx] = _add17i; } }
        } else {
            double _c18r = 0, _c18i = 0;
            _c18r = 2.0; _c18i = 0;
            double _fdiv19r = 0, _fdiv19i = 0;
            c_div(k, 0, _c18r, _c18i, &_fdiv19r, &_fdiv19i);
            _fdiv19r = floor(_fdiv19r); _fdiv19i = 0;
            double _c20r = 0, _c20i = 0;
            _c20r = 1.0; _c20i = 0;
            double _add21r = 0, _add21i = 0;
            _add21r = _fdiv19r + _c20r; _add21i = _fdiv19i + _c20i;
            double _attr22r = 0, _attr22i = 0;
            _attr22r = x1i; _attr22i = 0;
            double _mul23r = 0, _mul23i = 0;
            c_mul(_add21r, _add21i, _attr22r, _attr22i, &_mul23r, &_mul23i);
            p = _mul23r;
            double _c24r = 0, _c24i = 0;
            _c24r = 2.0; _c24i = 0;
            double _pow25r = 0, _pow25i = 0;
            c_mul(p, 0, p, 0, &_pow25r, &_pow25i);
            double _c26r = 0, _c26i = 0;
            _c26r = 0.0; _c26i = 1.0;
            double _mul27r = 0, _mul27i = 0;
            c_mul(_c26r, _c26i, p, 0, &_mul27r, &_mul27i);
            double _attr28r = 0, _attr28i = 0;
            _attr28r = x2r; _attr28i = 0;
            double _mul29r = 0, _mul29i = 0;
            c_mul(_mul27r, _mul27i, _attr28r, _attr28i, &_mul29r, &_mul29i);
            double _sub30r = 0, _sub30i = 0;
            _sub30r = _pow25r - _mul29r; _sub30i = _pow25i - _mul29i;
            { int _idx = k; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _sub30r; cIm[_idx] = _sub30i; } }
        }
    }
    for (int _si = 0; _si < 36; _si++) {
        int _si_idx = _si + 0;
        double _call31r = 0, _call31i = 0;
        /* WARNING: unhandled call Attribute(value=Subscript(value=Name(id='cf', ctx=Load()), slice=Slice(lower=Constant(value=1), step=Constant(value=2)), ctx=Load()), attr='conj', ctx=Load()) */
        cRe[_si_idx] += _call31r; cIm[_si_idx] += _call31i;
    }
    for (int _si = 0; _si < 35; _si++) {
        int _si_idx = _si + 1;
        double _call32r = 0, _call32i = 0;
        /* WARNING: unhandled call Attribute(value=Subscript(value=Name(id='cf', ctx=Load()), slice=Slice(step=Constant(value=2)), ctx=Load()), attr='conj', ctx=Load()) */
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_creative9_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _unk1r = 0, _unk1i = 0;
    /* WARNING: unhandled node BoolOp(op=Or(), values=[Call(func=Attribute(value=Attribute(value=Name(id='ps', ctx=Load()), attr='poly', ctx=Load()), attr='get', ctx=Load()), args=[Constant(value='n')]), Constant(value=71)]) */
    double n = _unk1r; /* +_unk1ii */
    for (int k = 0; k < (int)(n); k++) {
        double _c2r = 0, _c2i = 0;
        _c2r = 1.0; _c2i = 0;
        double _add3r = 0, _add3i = 0;
        _add3r = k + _c2r; _add3i = 0 + _c2i;
        double _np4r = 0, _np4i = 0;
        /* WARNING: unhandled np.phase */
        double _mul5r = 0, _mul5i = 0;
        c_mul(_add3r, _add3i, _np4r, _np4i, &_mul5r, &_mul5i);
        double freq_t1 = _mul5r; /* +_mul5ii */
        double _c6r = 0, _c6i = 0;
        _c6r = 1.0; _c6i = 0;
        double _add7r = 0, _add7i = 0;
        _add7r = k + _c6r; _add7i = 0 + _c6i;
        double _np8r = 0, _np8i = 0;
        /* WARNING: unhandled np.phase */
        double _mul9r = 0, _mul9i = 0;
        c_mul(_add7r, _add7i, _np8r, _np8i, &_mul9r, &_mul9i);
        double freq_t2 = _mul9r; /* +_mul9ii */
        double _sin10r = 0, _sin10i = 0;
        c_sin(freq_t1, 0, &_sin10r, &_sin10i);
        double _c11r = 0, _c11i = 0;
        _c11r = 0.0; _c11i = 1.0;
        double _cos12r = 0, _cos12i = 0;
        c_cos(freq_t2, 0, &_cos12r, &_cos12i);
        double _mul13r = 0, _mul13i = 0;
        c_mul(_c11r, _c11i, _cos12r, _cos12i, &_mul13r, &_mul13i);
        double _add14r = 0, _add14i = 0;
        _add14r = _sin10r + _mul13r; _add14i = _sin10i + _mul13i;
        double _mul15r = 0, _mul15i = 0;
        c_mul(x1r, x1i, x2r, x2i, &_mul15r, &_mul15i);
        double _abs16r = 0, _abs16i = 0;
        _abs16r = c_abs(_mul15r, _mul15i); _abs16i = 0;
        double _neg17r = 0, _neg17i = 0;
        _neg17r = -(_abs16r); _neg17i = -(_abs16i);
        double _mul18r = 0, _mul18i = 0;
        c_mul(_neg17r, _neg17i, k, 0, &_mul18r, &_mul18i);
        double _div19r = 0, _div19i = 0;
        c_div(_mul18r, _mul18i, n, 0, &_div19r, &_div19i);
        double _exp20r = 0, _exp20i = 0;
        c_exp2(_div19r, _div19i, &_exp20r, &_exp20i);
        double _mul21r = 0, _mul21i = 0;
        c_mul(_add14r, _add14i, _exp20r, _exp20i, &_mul21r, &_mul21i);
        { int _idx = k; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul21r; cIm[_idx] = _mul21i; } }
    }
    double _c22r = 0, _c22i = 0;
    _c22r = 1.0; _c22i = 0;
    double _sub23r = 0, _sub23i = 0;
    _sub23r = n - _c22r; _sub23i = 0 - _c22i;
    for (int k = 1; k < (int)(_sub23r); k++) {
        double _cf24r = 0, _cf24i = 0;
        { int _idx = (k - 1); if (_idx >= 0 && _idx < 36) { _cf24r = cRe[_idx]; _cf24i = cIm[_idx]; } }
        double _cf25r = 0, _cf25i = 0;
        { int _idx = (k + 1); if (_idx >= 0 && _idx < 36) { _cf25r = cRe[_idx]; _cf25i = cIm[_idx]; } }
        double _add26r = 0, _add26i = 0;
        _add26r = _cf24r + _cf25r; _add26i = _cf24i + _cf25i;
        double _c27r = 0, _c27i = 0;
        _c27r = 0.5; _c27i = 0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(_add26r, _add26i, _c27r, _c27i, &_mul28r, &_mul28i);
        double _add29r = 0, _add29i = 0;
        _add29r = x1r + x2r; _add29i = x1i + x2i;
        double _mul30r = 0, _mul30i = 0;
        c_mul(_mul28r, _mul28i, _add29r, _add29i, &_mul30r, &_mul30i);
        { int _idx = k; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul30r; cIm[_idx] = _mul30i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_creative10: too complex for auto-transpile, stubbed */
static void poly_creative10_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_neuralnet: too complex for auto-transpile, stubbed */
static void poly_neuralnet_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_neuralnet1: too complex for auto-transpile, stubbed */
static void poly_neuralnet1_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_ca: too complex for auto-transpile, stubbed */
static void poly_ca_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_quantum: too complex for auto-transpile, stubbed */
static void poly_quantum_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_topological_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _unk1r = 0, _unk1i = 0;
    /* WARNING: unhandled node BoolOp(op=Or(), values=[Call(func=Attribute(value=Attribute(value=Name(id='ps', ctx=Load()), attr='poly', ctx=Load()), attr='get', ctx=Load()), args=[Constant(value='n')]), Constant(value=71)]) */
    double n = _unk1r; /* +_unk1ii */
    double _attr2r = 0, _attr2i = 0;
    _attr2r = x1r; _attr2i = 0;
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x2i; _attr3i = 0;
    double _mul4r = 0, _mul4i = 0;
    c_mul(_attr2r, _attr2i, _attr3r, _attr3i, &_mul4r, &_mul4i);
    double _attr5r = 0, _attr5i = 0;
    _attr5r = x1i; _attr5i = 0;
    double _attr6r = 0, _attr6i = 0;
    _attr6r = x2r; _attr6i = 0;
    double _mul7r = 0, _mul7i = 0;
    c_mul(_attr5r, _attr5i, _attr6r, _attr6i, &_mul7r, &_mul7i);
    double _sub8r = 0, _sub8i = 0;
    _sub8r = _mul4r - _mul7r; _sub8i = _mul4i - _mul7i;
    double _abs9r = 0, _abs9i = 0;
    _abs9r = c_abs(_sub8r, _sub8i); _abs9i = 0;
    double _c10r = 0, _c10i = 0;
    _c10r = 11.0; _c10i = 0;
    double _mod11r = 0, _mod11i = 0;
    _mod11r = fmod(_abs9r, _c10r); _mod11i = 0;
    double _int12r = 0, _int12i = 0;
    _int12r = (int)(_mod11r); _int12i = 0;
    double winding = _int12r; /* +_int12ii */
    for (int k = 0; k < (int)(n); k++) {
        double _c13r = 0, _c13i = 0;
        _c13r = 0.0; _c13i = 0;
        double _c14r = 0, _c14i = 0;
        _c14r = 1.0; _c14i = 0;
        double _neg15r = 0, _neg15i = 0;
        _neg15r = -(_c14r); _neg15i = -(_c14i);
        double _mod16r = 0, _mod16i = 0;
        _mod16r = fmod(k, winding); _mod16i = 0;
        double _pow17r = 0, _pow17i = 0;
        c_powr(_neg15r, _neg15i, _mod16r, &_pow17r, &_pow17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 1.0; _c18i = 0;
        double _tern19r = 0, _tern19i = 0;
        if (winding > _c13r) { _tern19r = _pow17r; _tern19i = _pow17i; }
        else { _tern19r = _c18r; _tern19i = _c18i; }
        double sign = _tern19r; /* +_tern19ii */
        double _np20r = 0, _np20i = 0;
        /* WARNING: unhandled np.phase */
        double _mul21r = 0, _mul21i = 0;
        c_mul(k, 0, _np20r, _np20i, &_mul21r, &_mul21i);
        double _sin22r = 0, _sin22i = 0;
        c_sin(_mul21r, _mul21i, &_sin22r, &_sin22i);
        double _c23r = 0, _c23i = 0;
        _c23r = 0.0; _c23i = 1.0;
        double _np24r = 0, _np24i = 0;
        /* WARNING: unhandled np.phase */
        double _mul25r = 0, _mul25i = 0;
        c_mul(k, 0, _np24r, _np24i, &_mul25r, &_mul25i);
        double _cos26r = 0, _cos26i = 0;
        c_cos(_mul25r, _mul25i, &_cos26r, &_cos26i);
        double _mul27r = 0, _mul27i = 0;
        c_mul(_c23r, _c23i, _cos26r, _cos26i, &_mul27r, &_mul27i);
        double _add28r = 0, _add28i = 0;
        _add28r = _sin22r + _mul27r; _add28i = _sin22i + _mul27i;
        double twist = _add28r; /* +_add28ii */
        double _abs29r = 0, _abs29i = 0;
        _abs29r = c_abs(x1r, x1i); _abs29i = 0;
        double _c30r = 0, _c30i = 0;
        _c30r = 5.0; _c30i = 0;
        double _mod31r = 0, _mod31i = 0;
        _mod31r = fmod(k, _c30r); _mod31i = 0;
        double _pow32r = 0, _pow32i = 0;
        c_powr(_abs29r, _abs29i, _mod31r, &_pow32r, &_pow32i);
        double _abs33r = 0, _abs33i = 0;
        _abs33r = c_abs(x2r, x2i); _abs33i = 0;
        double _c34r = 0, _c34i = 0;
        _c34r = 3.0; _c34i = 0;
        double _mod35r = 0, _mod35i = 0;
        _mod35r = fmod(k, _c34r); _mod35i = 0;
        double _pow36r = 0, _pow36i = 0;
        c_powr(_abs33r, _abs33i, _mod35r, &_pow36r, &_pow36i);
        double _sub37r = 0, _sub37i = 0;
        _sub37r = _pow32r - _pow36r; _sub37i = _pow32i - _pow36i;
        double _mul38r = 0, _mul38i = 0;
        c_mul(sign, 0, _sub37r, _sub37i, &_mul38r, &_mul38i);
        double _mul39r = 0, _mul39i = 0;
        c_mul(_mul38r, _mul38i, twist, 0, &_mul39r, &_mul39i);
        { int _idx = k; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul39r; cIm[_idx] = _mul39i; } }
    }
    /* WARNING: dynamic slice assignment */
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* ERROR transpiling poly_biomorphic: could not convert string to float: 'A' */

/* ERROR transpiling poly_gravitational: No module named 'numpy' */

static void poly_sonic_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _unk1r = 0, _unk1i = 0;
    /* WARNING: unhandled node BoolOp(op=Or(), values=[Call(func=Attribute(value=Attribute(value=Name(id='ps', ctx=Load()), attr='poly', ctx=Load()), attr='get', ctx=Load()), args=[Constant(value='n')]), Constant(value=71)]) */
    double n = _unk1r; /* +_unk1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 440.0; _c2i = 0;
    double _abs3r = 0, _abs3i = 0;
    _abs3r = c_abs(x1r, x1i); _abs3i = 0;
    double _abs4r = 0, _abs4i = 0;
    _abs4r = c_abs(x1r, x1i); _abs4i = 0;
    double _c5r = 0, _c5i = 0;
    _c5r = 1.0; _c5i = 0;
    double _max6r = 0, _max6i = 0;
    _max6r = fmax(_abs4r, _c5r); _max6i = 0;
    double _div7r = 0, _div7i = 0;
    c_div(_abs3r, _abs3i, _max6r, _max6i, &_div7r, &_div7i);
    double _mul8r = 0, _mul8i = 0;
    c_mul(_c2r, _c2i, _div7r, _div7i, &_mul8r, &_mul8i);
    double carrier_freq = _mul8r; /* +_mul8ii */
    double _c9r = 0, _c9i = 0;
    _c9r = 440.0; _c9i = 0;
    double _abs10r = 0, _abs10i = 0;
    _abs10r = c_abs(x2r, x2i); _abs10i = 0;
    double _abs11r = 0, _abs11i = 0;
    _abs11r = c_abs(x2r, x2i); _abs11i = 0;
    double _c12r = 0, _c12i = 0;
    _c12r = 1.0; _c12i = 0;
    double _max13r = 0, _max13i = 0;
    _max13r = fmax(_abs11r, _c12r); _max13i = 0;
    double _div14r = 0, _div14i = 0;
    c_div(_abs10r, _abs10i, _max13r, _max13i, &_div14r, &_div14i);
    double _mul15r = 0, _mul15i = 0;
    c_mul(_c9r, _c9i, _div14r, _div14i, &_mul15r, &_mul15i);
    double mod_freq = _mul15r; /* +_mul15ii */
    double _c16r = 0, _c16i = 0;
    _c16r = 10.0; _c16i = 0;
    double _sub17r = 0, _sub17i = 0;
    _sub17r = x1r - x2r; _sub17i = x1i - x2i;
    double _abs18r = 0, _abs18i = 0;
    _abs18r = c_abs(_sub17r, _sub17i); _abs18i = 0;
    double _mul19r = 0, _mul19i = 0;
    c_mul(_c16r, _c16i, _abs18r, _abs18i, &_mul19r, &_mul19i);
    double _abs20r = 0, _abs20i = 0;
    _abs20r = c_abs(x1r, x1i); _abs20i = 0;
    double _abs21r = 0, _abs21i = 0;
    _abs21r = c_abs(x2r, x2i); _abs21i = 0;
    double _add22r = 0, _add22i = 0;
    _add22r = _abs20r + _abs21r; _add22i = _abs20i + _abs21i;
    double _c23r = 0, _c23i = 0;
    _c23r = 1e-06; _c23i = 0;
    double _add24r = 0, _add24i = 0;
    _add24r = _add22r + _c23r; _add24i = _add22i + _c23i;
    double _div25r = 0, _div25i = 0;
    c_div(_mul19r, _mul19i, _add24r, _add24i, &_div25r, &_div25i);
    double mod_index = _div25r; /* +_div25ii */
    for (int k = 0; k < (int)(n); k++) {
        double _div26r = 0, _div26i = 0;
        c_div(k, 0, n, 0, &_div26r, &_div26i);
        double t = _div26r; /* +_div26ii */
        double _c27r = 0, _c27i = 0;
        _c27r = 2.0; _c27i = 0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(_c27r, _c27i, M_PI, 0, &_mul28r, &_mul28i);
        double _mul29r = 0, _mul29i = 0;
        c_mul(_mul28r, _mul28i, mod_freq, 0, &_mul29r, &_mul29i);
        double _mul30r = 0, _mul30i = 0;
        c_mul(_mul29r, _mul29i, t, 0, &_mul30r, &_mul30i);
        double _sin31r = 0, _sin31i = 0;
        c_sin(_mul30r, _mul30i, &_sin31r, &_sin31i);
        double _mul32r = 0, _mul32i = 0;
        c_mul(mod_index, 0, _sin31r, _sin31i, &_mul32r, &_mul32i);
        double mod = _mul32r; /* +_mul32ii */
        double _c33r = 0, _c33i = 0;
        _c33r = 2.0; _c33i = 0;
        double _mul34r = 0, _mul34i = 0;
        c_mul(_c33r, _c33i, M_PI, 0, &_mul34r, &_mul34i);
        double _mul35r = 0, _mul35i = 0;
        c_mul(_mul34r, _mul34i, carrier_freq, 0, &_mul35r, &_mul35i);
        double _mul36r = 0, _mul36i = 0;
        c_mul(_mul35r, _mul35i, t, 0, &_mul36r, &_mul36i);
        double _add37r = 0, _add37i = 0;
        _add37r = _mul36r + mod; _add37i = _mul36i + 0;
        double _sin38r = 0, _sin38i = 0;
        c_sin(_add37r, _add37i, &_sin38r, &_sin38i);
        double wave = _sin38r; /* +_sin38ii */
        double _c39r = 0, _c39i = 0;
        _c39r = 0.0; _c39i = 1.0;
        double _c40r = 0, _c40i = 0;
        _c40r = 2.0; _c40i = 0;
        double _mul41r = 0, _mul41i = 0;
        c_mul(_c40r, _c40i, M_PI, 0, &_mul41r, &_mul41i);
        double _mul42r = 0, _mul42i = 0;
        c_mul(_mul41r, _mul41i, carrier_freq, 0, &_mul42r, &_mul42i);
        double _mul43r = 0, _mul43i = 0;
        c_mul(_mul42r, _mul42i, t, 0, &_mul43r, &_mul43i);
        double _add44r = 0, _add44i = 0;
        _add44r = _mul43r + mod; _add44i = _mul43i + 0;
        double _cos45r = 0, _cos45i = 0;
        c_cos(_add44r, _add44i, &_cos45r, &_cos45i);
        double _mul46r = 0, _mul46i = 0;
        c_mul(_c39r, _c39i, _cos45r, _cos45i, &_mul46r, &_mul46i);
        double _add47r = 0, _add47i = 0;
        _add47r = wave + _mul46r; _add47i = 0 + _mul46i;
        { int _idx = k; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _add47r; cIm[_idx] = _add47i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_cryptic_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _unk1r = 0, _unk1i = 0;
    /* WARNING: unhandled node BoolOp(op=Or(), values=[Call(func=Attribute(value=Attribute(value=Name(id='ps', ctx=Load()), attr='poly', ctx=Load()), attr='get', ctx=Load()), args=[Constant(value='n')]), Constant(value=71)]) */
    double n = _unk1r; /* +_unk1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 2.0; _c2i = 0;
    double _c3r = 0, _c3i = 0;
    _c3r = 256.0; _c3i = 0;
    double _pow4r = 0, _pow4i = 0;
    c_powr(_c2r, _c2i, 256.0, &_pow4r, &_pow4i);
    double _c5r = 0, _c5i = 0;
    _c5r = 2.0; _c5i = 0;
    double _c6r = 0, _c6i = 0;
    _c6r = 32.0; _c6i = 0;
    double _pow7r = 0, _pow7i = 0;
    c_powr(_c5r, _c5i, 32.0, &_pow7r, &_pow7i);
    double _sub8r = 0, _sub8i = 0;
    _sub8r = _pow4r - _pow7r; _sub8i = _pow4i - _pow7i;
    double _c9r = 0, _c9i = 0;
    _c9r = 977.0; _c9i = 0;
    double _sub10r = 0, _sub10i = 0;
    _sub10r = _sub8r - _c9r; _sub10i = _sub8i - _c9i;
    double p = _sub10r; /* +_sub10ii */
    double _abs11r = 0, _abs11i = 0;
    _abs11r = c_abs(x1r, x1i); _abs11i = 0;
    double _c12r = 0, _c12i = 0;
    _c12r = 1000000.0; _c12i = 0;
    double _mul13r = 0, _mul13i = 0;
    c_mul(_abs11r, _abs11i, _c12r, _c12i, &_mul13r, &_mul13i);
    double _int14r = 0, _int14i = 0;
    _int14r = (int)(_mul13r); _int14i = 0;
    double _mod15r = 0, _mod15i = 0;
    _mod15r = fmod(_int14r, p); _mod15i = 0;
    double a = _mod15r; /* +_mod15ii */
    double _abs16r = 0, _abs16i = 0;
    _abs16r = c_abs(x2r, x2i); _abs16i = 0;
    double _c17r = 0, _c17i = 0;
    _c17r = 1000000.0; _c17i = 0;
    double _mul18r = 0, _mul18i = 0;
    c_mul(_abs16r, _abs16i, _c17r, _c17i, &_mul18r, &_mul18i);
    double _int19r = 0, _int19i = 0;
    _int19r = (int)(_mul18r); _int19i = 0;
    double _mod20r = 0, _mod20i = 0;
    _mod20r = fmod(_int19r, p); _mod20i = 0;
    double b = _mod20r; /* +_mod20ii */
    double x = a;
    double y = b;
    for (int k = 0; k < (int)(n); k++) {
        double _c21r = 0, _c21i = 0;
        _c21r = 3.0; _c21i = 0;
        double _c22r = 0, _c22i = 0;
        _c22r = 2.0; _c22i = 0;
        double _pow23r = 0, _pow23i = 0;
        c_mul(x, 0, x, 0, &_pow23r, &_pow23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_c21r, _c21i, _pow23r, _pow23i, &_mul24r, &_mul24i);
        double _add25r = 0, _add25i = 0;
        _add25r = _mul24r + a; _add25i = _mul24i + 0;
        double _call26r = 0, _call26i = 0;
        /* WARNING: unhandled call Name(id='pow', ctx=Load()) */
        double _mul27r = 0, _mul27i = 0;
        c_mul(_add25r, _add25i, _call26r, _call26i, &_mul27r, &_mul27i);
        double _mod28r = 0, _mod28i = 0;
        _mod28r = fmod(_mul27r, p); _mod28i = 0;
        double s = _mod28r; /* +_mod28ii */
        double _c29r = 0, _c29i = 0;
        _c29r = 2.0; _c29i = 0;
        double _pow30r = 0, _pow30i = 0;
        c_mul(s, 0, s, 0, &_pow30r, &_pow30i);
        double _c31r = 0, _c31i = 0;
        _c31r = 2.0; _c31i = 0;
        double _mul32r = 0, _mul32i = 0;
        c_mul(_c31r, _c31i, x, 0, &_mul32r, &_mul32i);
        double _sub33r = 0, _sub33i = 0;
        _sub33r = _pow30r - _mul32r; _sub33i = _pow30i - _mul32i;
        double _mod34r = 0, _mod34i = 0;
        _mod34r = fmod(_sub33r, p); _mod34i = 0;
        double x_new = _mod34r; /* +_mod34ii */
        double _sub35r = 0, _sub35i = 0;
        _sub35r = x - x_new; _sub35i = 0 - 0;
        double _mul36r = 0, _mul36i = 0;
        c_mul(s, 0, _sub35r, _sub35i, &_mul36r, &_mul36i);
        double _sub37r = 0, _sub37i = 0;
        _sub37r = _mul36r - y; _sub37i = _mul36i - 0;
        double _mod38r = 0, _mod38i = 0;
        _mod38r = fmod(_sub37r, p); _mod38i = 0;
        double y_new = _mod38r; /* +_mod38ii */
        x = x_new;
        y = y_new;
        double _div39r = 0, _div39i = 0;
        c_div(x, 0, p, 0, &_div39r, &_div39i);
        double _div40r = 0, _div40i = 0;
        c_div(y, 0, p, 0, &_div40r, &_div40i);
        double _cplx41r = 0, _cplx41i = 0;
        _cplx41r = _div39r; _cplx41i = _div40r;
        double _c42r = 0, _c42i = 0;
        _c42r = 1.0; _c42i = 0;
        double _neg43r = 0, _neg43i = 0;
        _neg43r = -(_c42r); _neg43i = -(_c42i);
        double _pow44r = 0, _pow44i = 0;
        c_powr(_neg43r, _neg43i, k, &_pow44r, &_pow44i);
        double _mul45r = 0, _mul45i = 0;
        c_mul(_cplx41r, _cplx41i, _pow44r, _pow44i, &_mul45r, &_mul45i);
        { int _idx = k; if (_idx >= 0 && _idx < 36) { cRe[_idx] = _mul45r; cIm[_idx] = _mul45i; } }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_holographic: too complex for auto-transpile, stubbed */
static void poly_holographic_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_chaosmorph_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _unk1r = 0, _unk1i = 0;
    /* WARNING: unhandled node BoolOp(op=Or(), values=[Call(func=Attribute(value=Attribute(value=Name(id='ps', ctx=Load()), attr='poly', ctx=Load()), attr='get', ctx=Load()), args=[Constant(value='n')]), Constant(value=71)]) */
    double n = _unk1r; /* +_unk1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 0.2; _c2i = 0;
    double _c3r = 0, _c3i = 0;
    _c3r = 0.1; _c3i = 0;
    double _abs4r = 0, _abs4i = 0;
    _abs4r = c_abs(x1r, x1i); _abs4i = 0;
    double _mul5r = 0, _mul5i = 0;
    c_mul(_c3r, _c3i, _abs4r, _abs4i, &_mul5r, &_mul5i);
    double _add6r = 0, _add6i = 0;
    _add6r = _c2r + _mul5r; _add6i = _c2i + _mul5i;
    double a = _add6r; /* +_add6ii */
    double _c7r = 0, _c7i = 0;
    _c7r = 0.2; _c7i = 0;
    double _c8r = 0, _c8i = 0;
    _c8r = 0.1; _c8i = 0;
    double _abs9r = 0, _abs9i = 0;
    _abs9r = c_abs(x2r, x2i); _abs9i = 0;
    double _mul10r = 0, _mul10i = 0;
    c_mul(_c8r, _c8i, _abs9r, _abs9i, &_mul10r, &_mul10i);
    double _add11r = 0, _add11i = 0;
    _add11r = _c7r + _mul10r; _add11i = _c7i + _mul10i;
    double b = _add11r; /* +_add11ii */
    double _c12r = 0, _c12i = 0;
    _c12r = 5.7; _c12i = 0;
    double _np13r = 0, _np13i = 0;
    /* WARNING: unhandled np.phase */
    double _add14r = 0, _add14i = 0;
    _add14r = _c12r + _np13r; _add14i = _c12i + _np13i;
    double c = _add14r; /* +_add14ii */
    double _c15r = 0, _c15i = 0;
    _c15r = 0.1; _c15i = 0;
    double x = _c15r;
    double _c16r = 0, _c16i = 0;
    _c16r = 0.0; _c16i = 0;
    double y = _c16r;
    double _c17r = 0, _c17i = 0;
    _c17r = 0.0; _c17i = 0;
    double z = _c17r;
    double _c18r = 0, _c18i = 0;
    _c18r = 10.0; _c18i = 0;
    double _mul19r = 0, _mul19i = 0;
    c_mul(n, 0, _c18r, _c18i, &_mul19r, &_mul19i);
    for (int k = 0; k < (int)(_mul19r); k++) {
        double _neg20r = 0, _neg20i = 0;
        _neg20r = -(y); _neg20i = -(0);
        double _sub21r = 0, _sub21i = 0;
        _sub21r = _neg20r - z; _sub21i = _neg20i - 0;
        double dx = _sub21r; /* +_sub21ii */
        double _mul22r = 0, _mul22i = 0;
        c_mul(a, 0, y, 0, &_mul22r, &_mul22i);
        double _add23r = 0, _add23i = 0;
        _add23r = x + _mul22r; _add23i = 0 + _mul22i;
        double dy = _add23r; /* +_add23ii */
        double _sub24r = 0, _sub24i = 0;
        _sub24r = x - c; _sub24i = 0 - 0;
        double _mul25r = 0, _mul25i = 0;
        c_mul(z, 0, _sub24r, _sub24i, &_mul25r, &_mul25i);
        double _add26r = 0, _add26i = 0;
        _add26r = b + _mul25r; _add26i = 0 + _mul25i;
        double dz = _add26r; /* +_add26ii */
        double _c27r = 0, _c27i = 0;
        _c27r = 0.01; _c27i = 0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(_c27r, _c27i, dx, 0, &_mul28r, &_mul28i);
        x += _mul28r;
        double _c29r = 0, _c29i = 0;
        _c29r = 0.01; _c29i = 0;
        double _mul30r = 0, _mul30i = 0;
        c_mul(_c29r, _c29i, dy, 0, &_mul30r, &_mul30i);
        y += _mul30r;
        double _c31r = 0, _c31i = 0;
        _c31r = 0.01; _c31i = 0;
        double _mul32r = 0, _mul32i = 0;
        c_mul(_c31r, _c31i, dz, 0, &_mul32r, &_mul32i);
        z += _mul32r;
        double _c33r = 0, _c33i = 0;
        _c33r = 5.0; _c33i = 0;
        double _mul34r = 0, _mul34i = 0;
        c_mul(n, 0, _c33r, _c33i, &_mul34r, &_mul34i);
        if (k >= _mul34r) {
            double _cplx35r = 0, _cplx35i = 0;
            _cplx35r = x; _cplx35i = y;
            double _c36r = 0, _c36i = 0;
            _c36r = 0.1; _c36i = 0;
            double _neg37r = 0, _neg37i = 0;
            _neg37r = -(_c36r); _neg37i = -(_c36i);
            double _mul38r = 0, _mul38i = 0;
            c_mul(_neg37r, _neg37i, z, 0, &_mul38r, &_mul38i);
            double _exp39r = 0, _exp39i = 0;
            c_exp2(_mul38r, _mul38i, &_exp39r, &_exp39i);
            double _mul40r = 0, _mul40i = 0;
            c_mul(_cplx35r, _cplx35i, _exp39r, _exp39i, &_mul40r, &_mul40i);
            cRe[(k % (int)(n))] += _mul40r; cIm[(k % (int)(n))] += _mul40i;
        }
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_quasicrystal_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _unk1r = 0, _unk1i = 0;
    /* WARNING: unhandled node BoolOp(op=Or(), values=[Call(func=Attribute(value=Attribute(value=Name(id='ps', ctx=Load()), attr='poly', ctx=Load()), attr='get', ctx=Load()), args=[Constant(value='n')]), Constant(value=71)]) */
    double n = _unk1r; /* +_unk1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _c3r = 0, _c3i = 0;
    _c3r = 5.0; _c3i = 0;
    double _sqrt4r = 0, _sqrt4i = 0;
    c_powr(_c3r, _c3i, 0.5, &_sqrt4r, &_sqrt4i);
    double _add5r = 0, _add5i = 0;
    _add5r = _c2r + _sqrt4r; _add5i = _c2i + _sqrt4i;
    double _c6r = 0, _c6i = 0;
    _c6r = 2.0; _c6i = 0;
    double _div7r = 0, _div7i = 0;
    c_div(_add5r, _add5i, _c6r, _c6i, &_div7r, &_div7i);
    double phi = _div7r; /* +_div7ii */
    double _c8r = 0, _c8i = 0;
    _c8r = 2.0; _c8i = 0;
    double _mul9r = 0, _mul9i = 0;
    c_mul(0, 0, _c8r, _c8i, &_mul9r, &_mul9i);
    double _mul10r = 0, _mul10i = 0;
    c_mul(_mul9r, _mul9i, M_PI, 0, &_mul10r, &_mul10i);
    double _div11r = 0, _div11i = 0;
    c_div(_mul10r, _mul10i, phi, 0, &_div11r, &_div11i);
    double angles = _div11r; /* +_div11ii */
    double _div12r = 0, _div12i = 0;
    c_div(0, 0, n, 0, &_div12r, &_div12i);
    double _sqrt13r = 0, _sqrt13i = 0;
    c_powr(_div12r, _div12i, 0.5, &_sqrt13r, &_sqrt13i);
    double radii = _sqrt13r; /* +_sqrt13ii */
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* ERROR transpiling poly_neuroevolution: No module named 'numpy' */

/* ERROR transpiling poly_fluid: No module named 'numpy' */

/* poly_astro: too complex for auto-transpile, stubbed */
static void poly_astro_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_metamaterial_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _unk1r = 0, _unk1i = 0;
    /* WARNING: unhandled node BoolOp(op=Or(), values=[Call(func=Attribute(value=Attribute(value=Name(id='ps', ctx=Load()), attr='poly', ctx=Load()), attr='get', ctx=Load()), args=[Constant(value='n')]), Constant(value=71)]) */
    double n = _unk1r; /* +_unk1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 1.0; _c2i = 0;
    double _c3r = 0, _c3i = 0;
    _c3r = 0.5; _c3i = 0;
    double _c4r = 0, _c4i = 0;
    _c4r = 10.0; _c4i = 0;
    double _abs5r = 0, _abs5i = 0;
    _abs5r = c_abs(x1r, x1i); _abs5i = 0;
    double _mul6r = 0, _mul6i = 0;
    c_mul(_c4r, _c4i, _abs5r, _abs5i, &_mul6r, &_mul6i);
    double _mul7r = 0, _mul7i = 0;
    c_mul(_mul6r, _mul6i, M_PI, 0, &_mul7r, &_mul7i);
    double _mul8r = 0, _mul8i = 0;
    c_mul(_mul7r, _mul7i, 0, 0, &_mul8r, &_mul8i);
    double _div9r = 0, _div9i = 0;
    c_div(_mul8r, _mul8i, n, 0, &_div9r, &_div9i);
    double _sin10r = 0, _sin10i = 0;
    c_sin(_div9r, _div9i, &_sin10r, &_sin10i);
    double _sgn11r = 0, _sgn11i = 0;
    _sgn11r = (_sin10r > 0) ? 1.0 : (_sin10r < 0) ? -1.0 : 0.0; _sgn11i = 0;
    double _c12r = 0, _c12i = 0;
    _c12r = 8.0; _c12i = 0;
    double _abs13r = 0, _abs13i = 0;
    _abs13r = c_abs(x2r, x2i); _abs13i = 0;
    double _mul14r = 0, _mul14i = 0;
    c_mul(_c12r, _c12i, _abs13r, _abs13i, &_mul14r, &_mul14i);
    double _mul15r = 0, _mul15i = 0;
    c_mul(_mul14r, _mul14i, M_PI, 0, &_mul15r, &_mul15i);
    double _mul16r = 0, _mul16i = 0;
    c_mul(_mul15r, _mul15i, 0, 0, &_mul16r, &_mul16i);
    double _div17r = 0, _div17i = 0;
    c_div(_mul16r, _mul16i, n, 0, &_div17r, &_div17i);
    double _cos18r = 0, _cos18i = 0;
    c_cos(_div17r, _div17i, &_cos18r, &_cos18i);
    double _sgn19r = 0, _sgn19i = 0;
    _sgn19r = (_cos18r > 0) ? 1.0 : (_cos18r < 0) ? -1.0 : 0.0; _sgn19i = 0;
    double _add20r = 0, _add20i = 0;
    _add20r = _sgn11r + _sgn19r; _add20i = _sgn11i + _sgn19i;
    double _mul21r = 0, _mul21i = 0;
    c_mul(_c3r, _c3i, _add20r, _add20i, &_mul21r, &_mul21i);
    double _add22r = 0, _add22i = 0;
    _add22r = _c2r + _mul21r; _add22i = _c2i + _mul21i;
    double ε = _add22r; /* +_add22ii */
    double _np23r = 0, _np23i = 0;
    /* WARNING: unhandled np.diag */
    double _np24r = 0, _np24i = 0;
    /* WARNING: unhandled np.diag */
    double _sub25r = 0, _sub25i = 0;
    _sub25r = _np23r - _np24r; _sub25i = _np23i - _np24i;
    double _np26r = 0, _np26i = 0;
    /* WARNING: unhandled np.diag */
    double _sub27r = 0, _sub27i = 0;
    _sub27r = _sub25r - _np26r; _sub27i = _sub25i - _np26i;
    double M = _sub27r; /* +_sub27ii */
    double _call28r = 0, _call28i = 0;
    /* WARNING: unhandled call Attribute(value=Attribute(value=Name(id='np', ctx=Load()), attr='linalg', ctx=Load()), attr='eigvalsh', ctx=Load()) */
    double eigvals = _call28r; /* +_call28ii */
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_sync: too complex for auto-transpile, stubbed */
static void poly_sync_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_memristor_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _unk1r = 0, _unk1i = 0;
    /* WARNING: unhandled node BoolOp(op=Or(), values=[Call(func=Attribute(value=Attribute(value=Name(id='ps', ctx=Load()), attr='poly', ctx=Load()), attr='get', ctx=Load()), args=[Constant(value='n')]), Constant(value=71)]) */
    double n = _unk1r; /* +_unk1ii */
    double _np2r = 0, _np2i = 0;
    /* WARNING: unhandled np.outer */
    double W = _np2r; /* +_np2ii */
    double _attr3r = 0, _attr3i = 0;
    _attr3r = x1r; _attr3i = 0;
    double _np4r = 0, _np4i = 0;
    /* WARNING: unhandled np.linspace */
    double _sin5r = 0, _sin5i = 0;
    c_sin(_np4r, _np4i, &_sin5r, &_sin5i);
    double _mul6r = 0, _mul6i = 0;
    c_mul(_attr3r, _attr3i, _sin5r, _sin5i, &_mul6r, &_mul6i);
    double _attr7r = 0, _attr7i = 0;
    _attr7r = x2i; _attr7i = 0;
    double _np8r = 0, _np8i = 0;
    /* WARNING: unhandled np.linspace */
    double _cos9r = 0, _cos9i = 0;
    c_cos(_np8r, _np8i, &_cos9r, &_cos9i);
    double _mul10r = 0, _mul10i = 0;
    c_mul(_attr7r, _attr7i, _cos9r, _cos9i, &_mul10r, &_mul10i);
    double _add11r = 0, _add11i = 0;
    _add11r = _mul6r + _mul10r; _add11i = _mul6i + _mul10i;
    double V = _add11r; /* +_add11ii */
    for (int _ = 0; _ < 5; _++) {
        double _binop12r = 0, _binop12i = 0;
        /* WARNING: unhandled binop */
        double I = _binop12r; /* +_binop12ii */
        double _c13r = 0, _c13i = 0;
        _c13r = 0.01; _c13i = 0;
        double _np14r = 0, _np14i = 0;
        /* WARNING: unhandled np.outer */
        double _c15r = 0, _c15i = 0;
        _c15r = 0.1; _c15i = 0;
        double _mul16r = 0, _mul16i = 0;
        c_mul(_c15r, _c15i, W, 0, &_mul16r, &_mul16i);
        double _sub17r = 0, _sub17i = 0;
        _sub17r = _np14r - _mul16r; _sub17i = _np14i - _mul16i;
        double _mul18r = 0, _mul18i = 0;
        c_mul(_c13r, _c13i, _sub17r, _sub17i, &_mul18r, &_mul18i);
        W += _mul18r;
        double _tanh19r = 0, _tanh19i = 0;
        _tanh19r = tanh(I); _tanh19i = 0; /* approx real tanh */
        double _c20r = 0, _c20i = 0;
        _c20r = 0.0; _c20i = 1.0;
        double _ang21r = 0, _ang21i = 0;
        _ang21r = c_arg(I, 0); _ang21i = 0;
        double _mul22r = 0, _mul22i = 0;
        c_mul(_c20r, _c20i, _ang21r, _ang21i, &_mul22r, &_mul22i);
        double _exp23r = 0, _exp23i = 0;
        c_exp2(_mul22r, _mul22i, &_exp23r, &_exp23i);
        double _mul24r = 0, _mul24i = 0;
        c_mul(_tanh19r, _tanh19i, _exp23r, _exp23i, &_mul24r, &_mul24i);
        V = _mul24r;
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_swarm_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _unk1r = 0, _unk1i = 0;
    /* WARNING: unhandled node BoolOp(op=Or(), values=[Call(func=Attribute(value=Attribute(value=Name(id='ps', ctx=Load()), attr='poly', ctx=Load()), attr='get', ctx=Load()), args=[Constant(value='n')]), Constant(value=71)]) */
    double n = _unk1r; /* +_unk1ii */
    double _call2r = 0, _call2i = 0;
    /* WARNING: unhandled call Attribute(value=Attribute(value=Name(id='np', ctx=Load()), attr='random', ctx=Load()), attr='randn', ctx=Load()) */
    double _c3r = 0, _c3i = 0;
    _c3r = 0.0; _c3i = 1.0;
    double _call4r = 0, _call4i = 0;
    /* WARNING: unhandled call Attribute(value=Attribute(value=Name(id='np', ctx=Load()), attr='random', ctx=Load()), attr='randn', ctx=Load()) */
    double _mul5r = 0, _mul5i = 0;
    c_mul(_c3r, _c3i, _call4r, _call4i, &_mul5r, &_mul5i);
    double _add6r = 0, _add6i = 0;
    _add6r = _call2r + _mul5r; _add6i = _call2i + _mul5i;
    double pos = _add6r; /* +_add6ii */
    double _c7r = 0, _c7i = 0;
    _c7r = 0.1; _c7i = 0;
    double _call8r = 0, _call8i = 0;
    /* WARNING: unhandled call Attribute(value=Attribute(value=Name(id='np', ctx=Load()), attr='random', ctx=Load()), attr='randn', ctx=Load()) */
    double _c9r = 0, _c9i = 0;
    _c9r = 0.0; _c9i = 1.0;
    double _call10r = 0, _call10i = 0;
    /* WARNING: unhandled call Attribute(value=Attribute(value=Name(id='np', ctx=Load()), attr='random', ctx=Load()), attr='randn', ctx=Load()) */
    double _mul11r = 0, _mul11i = 0;
    c_mul(_c9r, _c9i, _call10r, _call10i, &_mul11r, &_mul11i);
    double _add12r = 0, _add12i = 0;
    _add12r = _call8r + _mul11r; _add12i = _call8i + _mul11i;
    double _mul13r = 0, _mul13i = 0;
    c_mul(_c7r, _c7i, _add12r, _add12i, &_mul13r, &_mul13i);
    double vel = _mul13r; /* +_mul13ii */
    double _call14r = 0, _call14i = 0;
    /* WARNING: unhandled call Attribute(value=Name(id='pos', ctx=Load()), attr='copy', ctx=Load()) */
    double pbest = _call14r; /* +_call14ii */
    double _np15r = 0, _np15i = 0;
    /* WARNING: unhandled np.mean */
    double gbest = _np15r; /* +_np15ii */
    for (int _ = 0; _ < 5; _++) {
        double _c16r = 0, _c16i = 0;
        _c16r = 0.5; _c16i = 0;
        double _mul17r = 0, _mul17i = 0;
        c_mul(_c16r, _c16i, vel, 0, &_mul17r, &_mul17i);
        double _c18r = 0, _c18i = 0;
        _c18r = 0.3; _c18i = 0;
        double _sub19r = 0, _sub19i = 0;
        _sub19r = pbest - pos; _sub19i = 0 - 0;
        double _mul20r = 0, _mul20i = 0;
        c_mul(_c18r, _c18i, _sub19r, _sub19i, &_mul20r, &_mul20i);
        double _add21r = 0, _add21i = 0;
        _add21r = _mul17r + _mul20r; _add21i = _mul17i + _mul20i;
        double _c22r = 0, _c22i = 0;
        _c22r = 0.2; _c22i = 0;
        double _sub23r = 0, _sub23i = 0;
        _sub23r = gbest - pos; _sub23i = 0 - 0;
        double _mul24r = 0, _mul24i = 0;
        c_mul(_c22r, _c22i, _sub23r, _sub23i, &_mul24r, &_mul24i);
        double _attr25r = 0, _attr25i = 0;
        _attr25r = x1r; _attr25i = 0;
        double _c26r = 0, _c26i = 0;
        _c26r = 0.0; _c26i = 1.0;
        double _attr27r = 0, _attr27i = 0;
        _attr27r = x2i; _attr27i = 0;
        double _mul28r = 0, _mul28i = 0;
        c_mul(_c26r, _c26i, _attr27r, _attr27i, &_mul28r, &_mul28i);
        double _add29r = 0, _add29i = 0;
        _add29r = _attr25r + _mul28r; _add29i = _attr25i + _mul28i;
        double _mul30r = 0, _mul30i = 0;
        c_mul(_mul24r, _mul24i, _add29r, _add29i, &_mul30r, &_mul30i);
        double _add31r = 0, _add31i = 0;
        _add31r = _add21r + _mul30r; _add31i = _add21i + _mul30i;
        vel = _add31r;
        pos += vel;
        double _np32r = 0, _np32i = 0;
        /* WARNING: unhandled np.where */
        pbest = _np32r;
        double _np33r = 0, _np33i = 0;
        /* WARNING: unhandled np.mean */
        gbest = _np33r;
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_cogniverse_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _unk1r = 0, _unk1i = 0;
    /* WARNING: unhandled node BoolOp(op=Or(), values=[Call(func=Attribute(value=Attribute(value=Name(id='ps', ctx=Load()), attr='poly', ctx=Load()), attr='get', ctx=Load()), args=[Constant(value='n')]), Constant(value=71)]) */
    double n = _unk1r; /* +_unk1ii */
    double _call2r = 0, _call2i = 0;
    /* WARNING: unhandled call Attribute(value=Attribute(value=Name(id='np', ctx=Load()), attr='random', ctx=Load()), attr='randn', ctx=Load()) */
    double _c3r = 0, _c3i = 0;
    _c3r = 0.0; _c3i = 1.0;
    double _call4r = 0, _call4i = 0;
    /* WARNING: unhandled call Attribute(value=Attribute(value=Name(id='np', ctx=Load()), attr='random', ctx=Load()), attr='randn', ctx=Load()) */
    double _mul5r = 0, _mul5i = 0;
    c_mul(_c3r, _c3i, _call4r, _call4i, &_mul5r, &_mul5i);
    double _add6r = 0, _add6i = 0;
    _add6r = _call2r + _mul5r; _add6i = _call2i + _mul5i;
    double v1 = _add6r; /* +_add6ii */
    double _call7r = 0, _call7i = 0;
    /* WARNING: unhandled call Attribute(value=Attribute(value=Name(id='np', ctx=Load()), attr='random', ctx=Load()), attr='randn', ctx=Load()) */
    double _c8r = 0, _c8i = 0;
    _c8r = 0.0; _c8i = 1.0;
    double _call9r = 0, _call9i = 0;
    /* WARNING: unhandled call Attribute(value=Attribute(value=Name(id='np', ctx=Load()), attr='random', ctx=Load()), attr='randn', ctx=Load()) */
    double _mul10r = 0, _mul10i = 0;
    c_mul(_c8r, _c8i, _call9r, _call9i, &_mul10r, &_mul10i);
    double _add11r = 0, _add11i = 0;
    _add11r = _call7r + _mul10r; _add11i = _call7i + _mul10i;
    double v2 = _add11r; /* +_add11ii */
    double _call12r = 0, _call12i = 0;
    /* WARNING: unhandled call Attribute(value=Attribute(value=Name(id='np', ctx=Load()), attr='linalg', ctx=Load()), attr='norm', ctx=Load()) */
    double _call13r = 0, _call13i = 0;
    /* WARNING: unhandled call Attribute(value=Attribute(value=Name(id='np', ctx=Load()), attr='linalg', ctx=Load()), attr='norm', ctx=Load()) */
    double _np14r = 0, _np14i = 0;
    /* WARNING: unhandled np.roll */
    double _mul15r = 0, _mul15i = 0;
    c_mul(v1, 0, _np14r, _np14i, &_mul15r, &_mul15i);
    double _c16r = 0, _c16i = 0;
    _c16r = 0.0; _c16i = 1.0;
    double _np17r = 0, _np17i = 0;
    /* WARNING: unhandled np.phase */
    double _mul18r = 0, _mul18i = 0;
    c_mul(_c16r, _c16i, _np17r, _np17i, &_mul18r, &_mul18i);
    double _exp19r = 0, _exp19i = 0;
    c_exp2(_mul18r, _mul18i, &_exp19r, &_exp19i);
    double _mul20r = 0, _mul20i = 0;
    c_mul(_mul15r, _mul15i, _exp19r, _exp19i, &_mul20r, &_mul20i);
    double bound = _mul20r; /* +_mul20ii */
    double _np21r = 0, _np21i = 0;
    /* WARNING: unhandled np.roll */
    double _add22r = 0, _add22i = 0;
    _add22r = bound + _np21r; _add22i = 0 + _np21i;
    double _c23r = 0, _c23i = 0;
    _c23r = 2.0; _c23i = 0;
    double _div24r = 0, _div24i = 0;
    c_div(_add22r, _add22i, _c23r, _c23i, &_div24r, &_div24i);
    double bundled = _div24r; /* +_div24ii */
    for (int _ = 0; _ < 3; _++) {
        double _call25r = 0, _call25i = 0;
        /* WARNING: unhandled call Attribute(value=Call(func=Attribute(value=Attribute(value=Name(id='np', ctx=Load()), attr='fft', ctx=Load()), attr='ifft', ctx=Load()), args=[BinOp(left=Call(func=Attribute(value=Attribute(value=Name(id='np', ctx=Load()), attr='fft', ctx=Load()), attr='fft', ctx=Load()), args=[Name(id='bundled', ctx=Load())]), op=Pow(), right=Constant(value=2))]), attr='conj', ctx=Load()) */
        bundled = _call25r;
    }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* ERROR transpiling poly_sandpile: No module named 'numpy' */

/* poly_spinglass: too complex for auto-transpile, stubbed */
static void poly_spinglass_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_letter_old_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double ascii = 2.0;
    double ro = 0.0;
    double io = 0.0;
    double norm = 0.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 0.0; _c1i = 1.0;
    double _mul2r = 0, _mul2i = 0;
    c_mul(_c1r, _c1i, io, 0, &_mul2r, &_mul2i);
    double _add3r = 0, _add3i = 0;
    _add3r = ro + _mul2r; _add3i = 0 + _mul2i;
    double offset = _add3r; /* +_add3ii */
    double _unk4r = 0, _unk4i = 0;
    /* WARNING: unhandled node JoinedStr(values=[Constant(value='b'), FormattedValue(value=Name(id='ascii', ctx=Load()), conversion=-1)]) */
    double key = _unk4r; /* +_unk4ii */
    double _call5r = 0, _call5i = 0;
    /* WARNING: unhandled call Attribute(value=Name(id='letters', ctx=Load()), attr='square', ctx=Load()) */
    double rts = _call5r; /* +_call5ii */
    double _np6r = 0, _np6i = 0;
    /* WARNING: unhandled np.max */
    double _c7r = 0, _c7i = 0;
    _c7r = 1.0; _c7i = 0;
    double _add8r = 0, _add8i = 0;
    _add8r = _np6r + _c7r; _add8i = _np6i + _c7i;
    double mrt = _add8r; /* +_add8ii */
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_letter_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double ascii = 2.0;
    double ro = 0.0;
    double io = 0.0;
    double factor = 1.0;
    double norm = 0.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 0.0; _c1i = 1.0;
    double _mul2r = 0, _mul2i = 0;
    c_mul(_c1r, _c1i, io, 0, &_mul2r, &_mul2i);
    double _add3r = 0, _add3i = 0;
    _add3r = ro + _mul2r; _add3i = 0 + _mul2i;
    double offset = _add3r; /* +_add3ii */
    double _unk4r = 0, _unk4i = 0;
    /* WARNING: unhandled node JoinedStr(values=[Constant(value='b'), FormattedValue(value=Name(id='ascii', ctx=Load()), conversion=-1)]) */
    double key = _unk4r; /* +_unk4ii */
    double _c5r = 0, _c5i = 0;
    _c5r = 0.0; _c5i = 1.0;
    double _mul6r = 0, _mul6i = 0;
    c_mul(_c5r, _c5i, ro, 0, &_mul6r, &_mul6i);
    double _add7r = 0, _add7i = 0;
    _add7r = ro + _mul6r; _add7i = 0 + _mul6i;
    offset = _add7r;
    double _call8r = 0, _call8i = 0;
    /* WARNING: unhandled call Attribute(value=Name(id='letters', ctx=Load()), attr='square', ctx=Load()) */
    double _mul9r = 0, _mul9i = 0;
    c_mul(_call8r, _call8i, factor, 0, &_mul9r, &_mul9i);
    double _add10r = 0, _add10i = 0;
    _add10r = _mul9r + offset; _add10i = _mul9i + 0;
    double rts = _add10r; /* +_add10ii */
    double _np11r = 0, _np11i = 0;
    /* WARNING: unhandled np.max */
    double _c12r = 0, _c12i = 0;
    _c12r = 1.0; _c12i = 0;
    double _add13r = 0, _add13i = 0;
    _add13r = _np11r + _c12r; _add13i = _np11i + _c12i;
    double mrt = _add13r; /* +_add13ii */
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_letter_2_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double ascii = 2.0;
    double ro = 0.0;
    double io = 0.0;
    double factor = 1.0;
    double norm = 0.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 0.0; _c1i = 1.0;
    double _mul2r = 0, _mul2i = 0;
    c_mul(_c1r, _c1i, io, 0, &_mul2r, &_mul2i);
    double _add3r = 0, _add3i = 0;
    _add3r = ro + _mul2r; _add3i = 0 + _mul2i;
    double offset = _add3r; /* +_add3ii */
    double _unk4r = 0, _unk4i = 0;
    /* WARNING: unhandled node JoinedStr(values=[Constant(value='b'), FormattedValue(value=Name(id='ascii', ctx=Load()), conversion=-1)]) */
    double key = _unk4r; /* +_unk4ii */
    double _c5r = 0, _c5i = 0;
    _c5r = 0.0; _c5i = 1.0;
    double _mul6r = 0, _mul6i = 0;
    c_mul(_c5r, _c5i, ro, 0, &_mul6r, &_mul6i);
    double _add7r = 0, _add7i = 0;
    _add7r = ro + _mul6r; _add7i = 0 + _mul6i;
    offset = _add7r;
    double _call8r = 0, _call8i = 0;
    /* WARNING: unhandled call Attribute(value=Name(id='letters', ctx=Load()), attr='square', ctx=Load()) */
    double _mul9r = 0, _mul9i = 0;
    c_mul(_call8r, _call8i, factor, 0, &_mul9r, &_mul9i);
    double _add10r = 0, _add10i = 0;
    _add10r = _mul9r + offset; _add10i = _mul9i + 0;
    double rts = _add10r; /* +_add10ii */
    double _np11r = 0, _np11i = 0;
    /* WARNING: unhandled np.max */
    double _c12r = 0, _c12i = 0;
    _c12r = 1.0; _c12i = 0;
    double _add13r = 0, _add13i = 0;
    _add13r = _np11r + _c12r; _add13i = _np11i + _c12i;
    double mrt = _add13r; /* +_add13ii */
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_letter1: too complex for auto-transpile, stubbed */
static void poly_letter1_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

static void poly_letter_roots_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_letter_path_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _sub1r = 0, _sub1i = 0;
    /* WARNING: unhandled subscript Subscript(value=Attribute(value=Name(id='letters', ctx=Load()), attr='FONTXY', ctx=Load()), slice=Constant(value='b2'), ctx=Load()) */
    double roots = _sub1r; /* +_sub1ii */
    double _c2r = 0, _c2i = 0;
    _c2r = 0.1; _c2i = 0;
    double _re3r = 0, _re3i = 0;
    _re3r = x1r; _re3i = 0;
    double _mul4r = 0, _mul4i = 0;
    c_mul(_c2r, _c2i, _re3r, _re3i, &_mul4r, &_mul4i);
    double _c5r = 0, _c5i = 0;
    _c5r = 0.0; _c5i = 1.0;
    double _c6r = 0, _c6i = 0;
    _c6r = 2.0; _c6i = 0;
    double _mul7r = 0, _mul7i = 0;
    c_mul(_c5r, _c5i, _c6r, _c6i, &_mul7r, &_mul7i);
    double _mul8r = 0, _mul8i = 0;
    c_mul(_mul7r, _mul7i, M_PI, 0, &_mul8r, &_mul8i);
    double _re9r = 0, _re9i = 0;
    _re9r = x2r; _re9i = 0;
    double _mul10r = 0, _mul10i = 0;
    c_mul(_mul8r, _mul8i, _re9r, _re9i, &_mul10r, &_mul10i);
    double _exp11r = 0, _exp11i = 0;
    c_exp2(_mul10r, _mul10i, &_exp11r, &_exp11i);
    double _mul12r = 0, _mul12i = 0;
    c_mul(_mul4r, _mul4i, _exp11r, _exp11i, &_mul12r, &_mul12i);
    double pert = _mul12r; /* +_mul12ii */
    double _c13r = 0, _c13i = 0;
    _c13r = 1.0; _c13i = 0;
    double _c14r = 0, _c14i = 0;
    _c14r = 2.0; _c14i = 0;
    double _mul15r = 0, _mul15i = 0;
    c_mul(_c14r, _c14i, M_PI, 0, &_mul15r, &_mul15i);
    double _mul16r = 0, _mul16i = 0;
    c_mul(_mul15r, _mul15i, x1r, x1i, &_mul16r, &_mul16i);
    double _mul17r = 0, _mul17i = 0;
    c_mul(_mul16r, _mul16i, 0, 0, &_mul17r, &_mul17i);
    double _sin18r = 0, _sin18i = 0;
    c_sin(_mul17r, _mul17i, &_sin18r, &_sin18i);
    double _mul19r = 0, _mul19i = 0;
    c_mul(_c13r, _c13i, _sin18r, _sin18i, &_mul19r, &_mul19i);
    double circles = _mul19r; /* +_mul19ii */
    double _np20r = 0, _np20i = 0;
    /* WARNING: unhandled np.flip */
    double _add21r = 0, _add21i = 0;
    _add21r = circles + _np20r; _add21i = 0 + _np20i;
    double symmetrized_circles = _add21r; /* +_add21ii */
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_letter2_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double _c1r = 0, _c1i = 0;
    _c1r = 0.1; _c1i = 0;
    double _re2r = 0, _re2i = 0;
    _re2r = x1r; _re2i = 0;
    double _mul3r = 0, _mul3i = 0;
    c_mul(_c1r, _c1i, _re2r, _re2i, &_mul3r, &_mul3i);
    double _c4r = 0, _c4i = 0;
    _c4r = 0.0; _c4i = 1.0;
    double _c5r = 0, _c5i = 0;
    _c5r = 2.0; _c5i = 0;
    double _mul6r = 0, _mul6i = 0;
    c_mul(_c4r, _c4i, _c5r, _c5i, &_mul6r, &_mul6i);
    double _mul7r = 0, _mul7i = 0;
    c_mul(_mul6r, _mul6i, M_PI, 0, &_mul7r, &_mul7i);
    double _re8r = 0, _re8i = 0;
    _re8r = x2r; _re8i = 0;
    double _mul9r = 0, _mul9i = 0;
    c_mul(_mul7r, _mul7i, _re8r, _re8i, &_mul9r, &_mul9i);
    double _exp10r = 0, _exp10i = 0;
    c_exp2(_mul9r, _mul9i, &_exp10r, &_exp10i);
    double _mul11r = 0, _mul11i = 0;
    c_mul(_mul3r, _mul3i, _exp10r, _exp10i, &_mul11r, &_mul11i);
    double pert = _mul11r; /* +_mul11ii */
    double _sub12r = 0, _sub12i = 0;
    /* WARNING: unhandled subscript Subscript(value=Attribute(value=Name(id='letters', ctx=Load()), attr='FONTXY', ctx=Load()), slice=Constant(value='x'), ctx=Load()) */
    double _c13r = 0, _c13i = 0;
    _c13r = 10.0; _c13i = 0;
    double _add14r = 0, _add14i = 0;
    _add14r = _sub12r + _c13r; _add14i = _sub12i + _c13i;
    double roots1 = _add14r; /* +_add14ii */
    double _c15r = 0, _c15i = 0;
    _c15r = 0.1; _c15i = 0;
    double _c16r = 0, _c16i = 0;
    _c16r = 2.0; _c16i = 0;
    double _mul17r = 0, _mul17i = 0;
    c_mul(_c16r, _c16i, M_PI, 0, &_mul17r, &_mul17i);
    double _mul18r = 0, _mul18i = 0;
    c_mul(_mul17r, _mul17i, x1r, x1i, &_mul18r, &_mul18i);
    double _mul19r = 0, _mul19i = 0;
    c_mul(_mul18r, _mul18i, 0, 0, &_mul19r, &_mul19i);
    double _sin20r = 0, _sin20i = 0;
    c_sin(_mul19r, _mul19i, &_sin20r, &_sin20i);
    double _mul21r = 0, _mul21i = 0;
    c_mul(_c15r, _c15i, _sin20r, _sin20i, &_mul21r, &_mul21i);
    double circles1 = _mul21r; /* +_mul21ii */
    double _np22r = 0, _np22i = 0;
    /* WARNING: unhandled np.flip */
    double _add23r = 0, _add23i = 0;
    _add23r = circles1 + _np22r; _add23i = 0 + _np22i;
    double symmetrized_circles1 = _add23r; /* +_add23ii */
    double _call24r = 0, _call24i = 0;
    /* WARNING: unhandled call Attribute(value=Call(func=Attribute(value=Name(id='np', ctx=Load()), attr='poly', ctx=Load()), args=[BinOp(left=Name(id='roots1', ctx=Load()), op=Add(), right=BinOp(left=Name(id='pert', ctx=Load()), op=Mult(), right=Name(id='symmetrized_circles1', ctx=Load())))]), attr='astype', ctx=Load()) */
    double cf1 = _call24r; /* +_call24ii */
    double _sub25r = 0, _sub25i = 0;
    /* WARNING: unhandled subscript Subscript(value=Attribute(value=Name(id='letters', ctx=Load()), attr='FONTXY', ctx=Load()), slice=Constant(value='v'), ctx=Load()) */
    double _c26r = 0, _c26i = 0;
    _c26r = 10.0; _c26i = 0;
    double _sub27r = 0, _sub27i = 0;
    _sub27r = _sub25r - _c26r; _sub27i = _sub25i - _c26i;
    double roots2 = _sub27r; /* +_sub27ii */
    double _c28r = 0, _c28i = 0;
    _c28r = 0.1; _c28i = 0;
    double _c29r = 0, _c29i = 0;
    _c29r = 2.0; _c29i = 0;
    double _mul30r = 0, _mul30i = 0;
    c_mul(_c29r, _c29i, M_PI, 0, &_mul30r, &_mul30i);
    double _mul31r = 0, _mul31i = 0;
    c_mul(_mul30r, _mul30i, x1r, x1i, &_mul31r, &_mul31i);
    double _mul32r = 0, _mul32i = 0;
    c_mul(_mul31r, _mul31i, 0, 0, &_mul32r, &_mul32i);
    double _sin33r = 0, _sin33i = 0;
    c_sin(_mul32r, _mul32i, &_sin33r, &_sin33i);
    double _mul34r = 0, _mul34i = 0;
    c_mul(_c28r, _c28i, _sin33r, _sin33i, &_mul34r, &_mul34i);
    double circles2 = _mul34r; /* +_mul34ii */
    double _np35r = 0, _np35i = 0;
    /* WARNING: unhandled np.flip */
    double _add36r = 0, _add36i = 0;
    _add36r = circles2 + _np35r; _add36i = 0 + _np35i;
    double symmetrized_circles2 = _add36r; /* +_add36ii */
    double _call37r = 0, _call37i = 0;
    /* WARNING: unhandled call Attribute(value=Call(func=Attribute(value=Name(id='np', ctx=Load()), attr='poly', ctx=Load()), args=[BinOp(left=Name(id='roots2', ctx=Load()), op=Add(), right=BinOp(left=Name(id='pert', ctx=Load()), op=Mult(), right=Name(id='symmetrized_circles2', ctx=Load())))]), attr='astype', ctx=Load()) */
    double cf2 = _call37r; /* +_call37ii */
    double _call38r = 0, _call38i = 0;
    /* WARNING: unhandled call Name(id='bimodal_skewed', ctx=Load()) */
    double a = _call38r; /* +_call38ii */
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* ERROR transpiling poly_lis2: No module named 'numpy' */

static void poly_chess_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double N = 7.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 2.0; _c1i = 0;
    double _mul2r = 0, _mul2i = 0;
    c_mul(_c1r, _c1i, M_PI, 0, &_mul2r, &_mul2i);
    double w = _mul2r; /* +_mul2ii */
    double _call3r = 0, _call3i = 0;
    /* WARNING: unhandled call Attribute(value=Attribute(value=Name(id='np', ctx=Load()), attr='random', ctx=Load()), attr='uniform', ctx=Load()) */
    double t = _call3r; /* +_call3ii */
    double _sin4r = 0, _sin4i = 0;
    c_sin(t, 0, &_sin4r, &_sin4i);
    double _np5r = 0, _np5i = 0;
    /* WARNING: unhandled np.tile */
    double _add6r = 0, _add6i = 0;
    _add6r = _sin4r + _np5r; _add6i = _sin4i + _np5i;
    double x = _add6r; /* +_add6ii */
    double _cos7r = 0, _cos7i = 0;
    c_cos(t, 0, &_cos7r, &_cos7i);
    double _np8r = 0, _np8i = 0;
    /* WARNING: unhandled np.repeat */
    double _add9r = 0, _add9i = 0;
    _add9r = _cos7r + _np8r; _add9i = _cos7i + _np8i;
    double y = _add9r; /* +_add9ii */
    double _c10r = 0, _c10i = 0;
    _c10r = 1.0; _c10i = 0;
    double _add11r = 0, _add11i = 0;
    _add11r = N + _c10r; _add11i = 0 + _c10i;
    double _c12r = 0, _c12i = 0;
    _c12r = 2.0; _c12i = 0;
    double _div13r = 0, _div13i = 0;
    c_div(_add11r, _add11i, _c12r, _c12i, &_div13r, &_div13i);
    double _sub14r = 0, _sub14i = 0;
    _sub14r = 0 - _div13r; _sub14i = 0 - _div13i;
    double _c15r = 0, _c15i = 0;
    _c15r = 0.0; _c15i = 1.0;
    double _c16r = 0, _c16i = 0;
    _c16r = 1.0; _c16i = 0;
    double _add17r = 0, _add17i = 0;
    _add17r = N + _c16r; _add17i = 0 + _c16i;
    double _c18r = 0, _c18i = 0;
    _c18r = 2.0; _c18i = 0;
    double _div19r = 0, _div19i = 0;
    c_div(_add17r, _add17i, _c18r, _c18i, &_div19r, &_div19i);
    double _mul20r = 0, _mul20i = 0;
    c_mul(_c15r, _c15i, _div19r, _div19i, &_mul20r, &_mul20i);
    double _sub21r = 0, _sub21i = 0;
    _sub21r = _sub14r - _mul20r; _sub21i = _sub14i - _mul20i;
    double curve = _sub21r; /* +_sub21ii */
    double _np22r = 0, _np22i = 0;
    /* WARNING: unhandled np.poly */
    double coeffs = _np22r; /* +_np22ii */
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_chess1_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double N = 7.0;
    double _c1r = 0, _c1i = 0;
    _c1r = 2.0; _c1i = 0;
    double _mul2r = 0, _mul2i = 0;
    c_mul(_c1r, _c1i, M_PI, 0, &_mul2r, &_mul2i);
    double w = _mul2r; /* +_mul2ii */
    double _call3r = 0, _call3i = 0;
    /* WARNING: unhandled call Attribute(value=Attribute(value=Name(id='np', ctx=Load()), attr='random', ctx=Load()), attr='uniform', ctx=Load()) */
    double t = _call3r; /* +_call3ii */
    double _sin4r = 0, _sin4i = 0;
    c_sin(t, 0, &_sin4r, &_sin4i);
    double _np5r = 0, _np5i = 0;
    /* WARNING: unhandled np.tile */
    double _add6r = 0, _add6i = 0;
    _add6r = _sin4r + _np5r; _add6i = _sin4i + _np5i;
    double x = _add6r; /* +_add6ii */
    double _cos7r = 0, _cos7i = 0;
    c_cos(t, 0, &_cos7r, &_cos7i);
    double _np8r = 0, _np8i = 0;
    /* WARNING: unhandled np.repeat */
    double _add9r = 0, _add9i = 0;
    _add9r = _cos7r + _np8r; _add9i = _cos7i + _np8i;
    double y = _add9r; /* +_add9ii */
    double _c10r = 0, _c10i = 0;
    _c10r = 1.0; _c10i = 0;
    double _add11r = 0, _add11i = 0;
    _add11r = N + _c10r; _add11i = 0 + _c10i;
    double _c12r = 0, _c12i = 0;
    _c12r = 2.0; _c12i = 0;
    double _div13r = 0, _div13i = 0;
    c_div(_add11r, _add11i, _c12r, _c12i, &_div13r, &_div13i);
    double _sub14r = 0, _sub14i = 0;
    _sub14r = 0 - _div13r; _sub14i = 0 - _div13i;
    double _c15r = 0, _c15i = 0;
    _c15r = 0.0; _c15i = 1.0;
    double _c16r = 0, _c16i = 0;
    _c16r = 1.0; _c16i = 0;
    double _add17r = 0, _add17i = 0;
    _add17r = N + _c16r; _add17i = 0 + _c16i;
    double _c18r = 0, _c18i = 0;
    _c18r = 2.0; _c18i = 0;
    double _div19r = 0, _div19i = 0;
    c_div(_add17r, _add17i, _c18r, _c18i, &_div19r, &_div19i);
    double _mul20r = 0, _mul20i = 0;
    c_mul(_c15r, _c15i, _div19r, _div19i, &_mul20r, &_mul20i);
    double _sub21r = 0, _sub21i = 0;
    _sub21r = _sub14r - _mul20r; _sub21i = _sub14i - _mul20i;
    double curve = _sub21r; /* +_sub21ii */
    double _np22r = 0, _np22i = 0;
    /* WARNING: unhandled np.poly */
    double coeffs = _np22r; /* +_np22ii */
    double cf1 = coeffs; /* +0i */
    double _np23r = 0, _np23i = 0;
    /* WARNING: unhandled np.pad */
    double cf2 = _np23r; /* +_np23ii */
    double _c24r = 0, _c24i = 0;
    _c24r = 0.0001; _c24i = 0;
    double _mul25r = 0, _mul25i = 0;
    c_mul(cf2, 0, _c24r, _c24i, &_mul25r, &_mul25i);
    double _add26r = 0, _add26i = 0;
    _add26r = cf1 + _mul25r; _add26i = 0 + _mul25i;
    double cf3 = _add26r; /* +_add26ii */
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

/* poly_chess2: too complex for auto-transpile, stubbed */
static void poly_chess2_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_chess3: too complex for auto-transpile, stubbed */
static void poly_chess3_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* ERROR transpiling poly_chess4: float() argument must be a string or a real number, not 'NoneType' */

/* poly_chess5_old: too complex for auto-transpile, stubbed */
static void poly_chess5_old_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_chess5: too complex for auto-transpile, stubbed */
static void poly_chess5_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* poly_path: too complex for auto-transpile, stubbed */
static void poly_path_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    (void)x1r; (void)x1i; (void)x2r; (void)x2i;
}

/* ERROR transpiling poly_o3_1: float() argument must be a string or a real number, not 'NoneType' */

/* ERROR transpiling poly_pacman: No module named 'numpy' */

static void poly_rnd_path1_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double i = 0.0;
    double a = 0;
    double b = 0;
    double cf_end = 0;
    double cf_start = 0;
    double _c1r = 0, _c1i = 0;
    _c1r = 0.0; _c1i = 0;
    if (i == _c1r) {
        double _unk2r = 0, _unk2i = 0;
        /* WARNING: unhandled node BoolOp(op=Or(), values=[Call(func=Attribute(value=Attribute(value=Name(id='ps', ctx=Load()), attr='poly', ctx=Load()), attr='get', ctx=Load()), args=[Constant(value='a')]), Constant(value=1)]) */
        a = _unk2r;
        double _unk3r = 0, _unk3i = 0;
        /* WARNING: unhandled node BoolOp(op=Or(), values=[Call(func=Attribute(value=Attribute(value=Name(id='ps', ctx=Load()), attr='poly', ctx=Load()), attr='get', ctx=Load()), args=[Constant(value='b')]), Constant(value=0)]) */
        b = _unk3r;
        double _call4r = 0, _call4i = 0;
        /* WARNING: unhandled call Attribute(value=Name(id='ps', ctx=Load()), attr='json2cvec', ctx=Load()) */
        cf_start = _call4r;
        double _np5r = 0, _np5i = 0;
        /* WARNING: unhandled np.poly */
        cf_end = _np5r;
    }
    double _c6r = 0, _c6i = 0;
    _c6r = 1.0; _c6i = 0;
    double _sub7r = 0, _sub7i = 0;
    _sub7r = _c6r - x1r; _sub7i = _c6i - x1i;
    double _attr8r = 0, _attr8i = 0;
    _attr8r = cf_start; _attr8i = 0;
    double _mul9r = 0, _mul9i = 0;
    c_mul(_sub7r, _sub7i, _attr8r, _attr8i, &_mul9r, &_mul9i);
    double _attr10r = 0, _attr10i = 0;
    _attr10r = cf_end; _attr10i = 0;
    double _mul11r = 0, _mul11i = 0;
    c_mul(x1r, x1i, _attr10r, _attr10i, &_mul11r, &_mul11i);
    double _add12r = 0, _add12i = 0;
    _add12r = _mul9r + _mul11r; _add12i = _mul9i + _mul11i;
    double real_part = _add12r; /* +_add12ii */
    double _c13r = 0, _c13i = 0;
    _c13r = 1.0; _c13i = 0;
    double _sub14r = 0, _sub14i = 0;
    _sub14r = _c13r - x2r; _sub14i = _c13i - x2i;
    double _attr15r = 0, _attr15i = 0;
    _attr15r = 0; _attr15i = 0;
    double _mul16r = 0, _mul16i = 0;
    c_mul(_sub14r, _sub14i, _attr15r, _attr15i, &_mul16r, &_mul16i);
    double _attr17r = 0, _attr17i = 0;
    _attr17r = 0; _attr17i = 0;
    double _mul18r = 0, _mul18i = 0;
    c_mul(x2r, x2i, _attr17r, _attr17i, &_mul18r, &_mul18i);
    double _add19r = 0, _add19i = 0;
    _add19r = _mul16r + _mul18r; _add19i = _mul16i + _mul18i;
    double imag_part = _add19r; /* +_add19ii */
    double _c20r = 0, _c20i = 0;
    _c20r = 0.0; _c20i = 1.0;
    double _mul21r = 0, _mul21i = 0;
    c_mul(_c20r, _c20i, imag_part, 0, &_mul21r, &_mul21i);
    double _add22r = 0, _add22i = 0;
    _add22r = real_part + _mul21r; _add22i = 0 + _mul21i;
    double coeffs = _add22r; /* +_add22ii */
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_rnd_path2_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double i = 0.0;
    double a = 0;
    double b = 0;
    double cf_end = 0;
    double cf_start = 0;
    double _c1r = 0, _c1i = 0;
    _c1r = 0.0; _c1i = 0;
    if (i == _c1r) {
        double _unk2r = 0, _unk2i = 0;
        /* WARNING: unhandled node BoolOp(op=Or(), values=[Call(func=Attribute(value=Attribute(value=Name(id='ps', ctx=Load()), attr='poly', ctx=Load()), attr='get', ctx=Load()), args=[Constant(value='a')]), Constant(value=1)]) */
        a = _unk2r;
        double _unk3r = 0, _unk3i = 0;
        /* WARNING: unhandled node BoolOp(op=Or(), values=[Call(func=Attribute(value=Attribute(value=Name(id='ps', ctx=Load()), attr='poly', ctx=Load()), attr='get', ctx=Load()), args=[Constant(value='b')]), Constant(value=0)]) */
        b = _unk3r;
        double _call4r = 0, _call4i = 0;
        /* WARNING: unhandled call Attribute(value=Name(id='ps', ctx=Load()), attr='json2cvec', ctx=Load()) */
        cf_start = _call4r;
        double _np5r = 0, _np5i = 0;
        /* WARNING: unhandled np.poly */
        cf_end = _np5r;
    }
    double _call6r = 0, _call6i = 0;
    /* WARNING: unhandled call Attribute(value=Attribute(value=Name(id='np', ctx=Load()), attr='random', ctx=Load()), attr='uniform', ctx=Load()) */
    double t = _call6r; /* +_call6ii */
    double _c7r = 0, _c7i = 0;
    _c7r = 1.0; _c7i = 0;
    double _mul8r = 0, _mul8i = 0;
    c_mul(x1r, x1i, t, 0, &_mul8r, &_mul8i);
    double _sub9r = 0, _sub9i = 0;
    _sub9r = _c7r - _mul8r; _sub9i = _c7i - _mul8i;
    double _attr10r = 0, _attr10i = 0;
    _attr10r = cf_start; _attr10i = 0;
    double _mul11r = 0, _mul11i = 0;
    c_mul(_sub9r, _sub9i, _attr10r, _attr10i, &_mul11r, &_mul11i);
    double _mul12r = 0, _mul12i = 0;
    c_mul(x1r, x1i, t, 0, &_mul12r, &_mul12i);
    double _attr13r = 0, _attr13i = 0;
    _attr13r = cf_end; _attr13i = 0;
    double _mul14r = 0, _mul14i = 0;
    c_mul(_mul12r, _mul12i, _attr13r, _attr13i, &_mul14r, &_mul14i);
    double _add15r = 0, _add15i = 0;
    _add15r = _mul11r + _mul14r; _add15i = _mul11i + _mul14i;
    double real_part = _add15r; /* +_add15ii */
    double _c16r = 0, _c16i = 0;
    _c16r = 1.0; _c16i = 0;
    double _mul17r = 0, _mul17i = 0;
    c_mul(x2r, x2i, t, 0, &_mul17r, &_mul17i);
    double _sub18r = 0, _sub18i = 0;
    _sub18r = _c16r - _mul17r; _sub18i = _c16i - _mul17i;
    double _attr19r = 0, _attr19i = 0;
    _attr19r = 0; _attr19i = 0;
    double _mul20r = 0, _mul20i = 0;
    c_mul(_sub18r, _sub18i, _attr19r, _attr19i, &_mul20r, &_mul20i);
    double _mul21r = 0, _mul21i = 0;
    c_mul(x2r, x2i, t, 0, &_mul21r, &_mul21i);
    double _attr22r = 0, _attr22i = 0;
    _attr22r = 0; _attr22i = 0;
    double _mul23r = 0, _mul23i = 0;
    c_mul(_mul21r, _mul21i, _attr22r, _attr22i, &_mul23r, &_mul23i);
    double _add24r = 0, _add24i = 0;
    _add24r = _mul20r + _mul23r; _add24i = _mul20i + _mul23i;
    double imag_part = _add24r; /* +_add24ii */
    double _c25r = 0, _c25i = 0;
    _c25r = 0.0; _c25i = 1.0;
    double _mul26r = 0, _mul26i = 0;
    c_mul(_c25r, _c25i, imag_part, 0, &_mul26r, &_mul26i);
    double _add27r = 0, _add27i = 0;
    _add27r = real_part + _mul26r; _add27i = 0 + _mul26i;
    double coeffs = _add27r; /* +_add27ii */
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_rnd_path3_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double i = 0.0;
    double a = 0;
    double b = 0;
    double cf_end = 0;
    double cf_start = 0;
    double _c1r = 0, _c1i = 0;
    _c1r = 0.0; _c1i = 0;
    if (i == _c1r) {
        double _unk2r = 0, _unk2i = 0;
        /* WARNING: unhandled node BoolOp(op=Or(), values=[Call(func=Attribute(value=Attribute(value=Name(id='ps', ctx=Load()), attr='poly', ctx=Load()), attr='get', ctx=Load()), args=[Constant(value='a')]), Constant(value=1)]) */
        a = _unk2r;
        double _unk3r = 0, _unk3i = 0;
        /* WARNING: unhandled node BoolOp(op=Or(), values=[Call(func=Attribute(value=Attribute(value=Name(id='ps', ctx=Load()), attr='poly', ctx=Load()), attr='get', ctx=Load()), args=[Constant(value='b')]), Constant(value=0)]) */
        b = _unk3r;
        double _call4r = 0, _call4i = 0;
        /* WARNING: unhandled call Attribute(value=Name(id='ps', ctx=Load()), attr='json2cvec', ctx=Load()) */
        cf_start = _call4r;
        double _np5r = 0, _np5i = 0;
        /* WARNING: unhandled np.poly */
        cf_end = _np5r;
    }
    double _call6r = 0, _call6i = 0;
    /* WARNING: unhandled call Attribute(value=Attribute(value=Name(id='np', ctx=Load()), attr='random', ctx=Load()), attr='uniform', ctx=Load()) */
    double ta = _call6r; /* +_call6ii */
    double _call7r = 0, _call7i = 0;
    /* WARNING: unhandled call Attribute(value=Attribute(value=Name(id='np', ctx=Load()), attr='random', ctx=Load()), attr='uniform', ctx=Load()) */
    double tb = _call7r; /* +_call7ii */
    double _c8r = 0, _c8i = 0;
    _c8r = 1.0; _c8i = 0;
    double _mul9r = 0, _mul9i = 0;
    c_mul(x1r, x1i, ta, 0, &_mul9r, &_mul9i);
    double _sub10r = 0, _sub10i = 0;
    _sub10r = _c8r - _mul9r; _sub10i = _c8i - _mul9i;
    double _attr11r = 0, _attr11i = 0;
    _attr11r = cf_start; _attr11i = 0;
    double _mul12r = 0, _mul12i = 0;
    c_mul(_sub10r, _sub10i, _attr11r, _attr11i, &_mul12r, &_mul12i);
    double _mul13r = 0, _mul13i = 0;
    c_mul(x1r, x1i, ta, 0, &_mul13r, &_mul13i);
    double _attr14r = 0, _attr14i = 0;
    _attr14r = cf_end; _attr14i = 0;
    double _mul15r = 0, _mul15i = 0;
    c_mul(_mul13r, _mul13i, _attr14r, _attr14i, &_mul15r, &_mul15i);
    double _add16r = 0, _add16i = 0;
    _add16r = _mul12r + _mul15r; _add16i = _mul12i + _mul15i;
    double real_part = _add16r; /* +_add16ii */
    double _c17r = 0, _c17i = 0;
    _c17r = 1.0; _c17i = 0;
    double _mul18r = 0, _mul18i = 0;
    c_mul(x2r, x2i, tb, 0, &_mul18r, &_mul18i);
    double _sub19r = 0, _sub19i = 0;
    _sub19r = _c17r - _mul18r; _sub19i = _c17i - _mul18i;
    double _attr20r = 0, _attr20i = 0;
    _attr20r = 0; _attr20i = 0;
    double _mul21r = 0, _mul21i = 0;
    c_mul(_sub19r, _sub19i, _attr20r, _attr20i, &_mul21r, &_mul21i);
    double _mul22r = 0, _mul22i = 0;
    c_mul(x2r, x2i, tb, 0, &_mul22r, &_mul22i);
    double _attr23r = 0, _attr23i = 0;
    _attr23r = 0; _attr23i = 0;
    double _mul24r = 0, _mul24i = 0;
    c_mul(_mul22r, _mul22i, _attr23r, _attr23i, &_mul24r, &_mul24i);
    double _add25r = 0, _add25i = 0;
    _add25r = _mul21r + _mul24r; _add25i = _mul21i + _mul24i;
    double imag_part = _add25r; /* +_add25ii */
    double _c26r = 0, _c26i = 0;
    _c26r = 0.0; _c26i = 1.0;
    double _mul27r = 0, _mul27i = 0;
    c_mul(_c26r, _c26i, imag_part, 0, &_mul27r, &_mul27i);
    double _add28r = 0, _add28i = 0;
    _add28r = real_part + _mul27r; _add28i = 0 + _mul27i;
    double coeffs = _add28r; /* +_add28ii */
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_rnd_path4_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double i = 0.0;
    double a = 0;
    double b = 0;
    double cf_end = 0;
    double cf_start = 0;
    double _c1r = 0, _c1i = 0;
    _c1r = 0.0; _c1i = 0;
    if (i == _c1r) {
        double _unk2r = 0, _unk2i = 0;
        /* WARNING: unhandled node BoolOp(op=Or(), values=[Call(func=Attribute(value=Attribute(value=Name(id='ps', ctx=Load()), attr='poly', ctx=Load()), attr='get', ctx=Load()), args=[Constant(value='a')]), Constant(value=1)]) */
        a = _unk2r;
        double _unk3r = 0, _unk3i = 0;
        /* WARNING: unhandled node BoolOp(op=Or(), values=[Call(func=Attribute(value=Attribute(value=Name(id='ps', ctx=Load()), attr='poly', ctx=Load()), attr='get', ctx=Load()), args=[Constant(value='b')]), Constant(value=0)]) */
        b = _unk3r;
        double _call4r = 0, _call4i = 0;
        /* WARNING: unhandled call Attribute(value=Name(id='ps', ctx=Load()), attr='json2cvec', ctx=Load()) */
        cf_start = _call4r;
        double _np5r = 0, _np5i = 0;
        /* WARNING: unhandled np.poly */
        cf_end = _np5r;
    }
    double _call6r = 0, _call6i = 0;
    /* WARNING: unhandled call Attribute(value=Attribute(value=Name(id='np', ctx=Load()), attr='random', ctx=Load()), attr='uniform', ctx=Load()) */
    double ta = _call6r; /* +_call6ii */
    double _call7r = 0, _call7i = 0;
    /* WARNING: unhandled call Attribute(value=Attribute(value=Name(id='np', ctx=Load()), attr='random', ctx=Load()), attr='uniform', ctx=Load()) */
    double tb = _call7r; /* +_call7ii */
    double _c8r = 0, _c8i = 0;
    _c8r = 1.0; _c8i = 0;
    double _mul9r = 0, _mul9i = 0;
    c_mul(x1r, x1i, ta, 0, &_mul9r, &_mul9i);
    double _sub10r = 0, _sub10i = 0;
    _sub10r = _c8r - _mul9r; _sub10i = _c8i - _mul9i;
    double _attr11r = 0, _attr11i = 0;
    _attr11r = cf_start; _attr11i = 0;
    double _mul12r = 0, _mul12i = 0;
    c_mul(_sub10r, _sub10i, _attr11r, _attr11i, &_mul12r, &_mul12i);
    double _mul13r = 0, _mul13i = 0;
    c_mul(x1r, x1i, ta, 0, &_mul13r, &_mul13i);
    double _attr14r = 0, _attr14i = 0;
    _attr14r = cf_end; _attr14i = 0;
    double _mul15r = 0, _mul15i = 0;
    c_mul(_mul13r, _mul13i, _attr14r, _attr14i, &_mul15r, &_mul15i);
    double _add16r = 0, _add16i = 0;
    _add16r = _mul12r + _mul15r; _add16i = _mul12i + _mul15i;
    double real_part = _add16r; /* +_add16ii */
    double _c17r = 0, _c17i = 0;
    _c17r = 1.0; _c17i = 0;
    double _mul18r = 0, _mul18i = 0;
    c_mul(x2r, x2i, tb, 0, &_mul18r, &_mul18i);
    double _sub19r = 0, _sub19i = 0;
    _sub19r = _c17r - _mul18r; _sub19i = _c17i - _mul18i;
    double _attr20r = 0, _attr20i = 0;
    _attr20r = 0; _attr20i = 0;
    double _mul21r = 0, _mul21i = 0;
    c_mul(_sub19r, _sub19i, _attr20r, _attr20i, &_mul21r, &_mul21i);
    double _mul22r = 0, _mul22i = 0;
    c_mul(x2r, x2i, tb, 0, &_mul22r, &_mul22i);
    double _attr23r = 0, _attr23i = 0;
    _attr23r = 0; _attr23i = 0;
    double _mul24r = 0, _mul24i = 0;
    c_mul(_mul22r, _mul22i, _attr23r, _attr23i, &_mul24r, &_mul24i);
    double _add25r = 0, _add25i = 0;
    _add25r = _mul21r + _mul24r; _add25i = _mul21i + _mul24i;
    double imag_part = _add25r; /* +_add25ii */
    double _c26r = 0, _c26i = 0;
    _c26r = 0.0; _c26i = 1.0;
    double _mul27r = 0, _mul27i = 0;
    c_mul(_c26r, _c26i, imag_part, 0, &_mul27r, &_mul27i);
    double _add28r = 0, _add28i = 0;
    _add28r = real_part + _mul27r; _add28i = 0 + _mul27i;
    double coeffs = _add28r; /* +_add28ii */
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_rnd_path5_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double i = 0.0;
    double a = 0;
    double b = 0;
    double c = 0;
    double cf_end = 0;
    double cf_start = 0;
    double _c1r = 0, _c1i = 0;
    _c1r = 0.0; _c1i = 0;
    if (i == _c1r) {
        double _unk2r = 0, _unk2i = 0;
        /* WARNING: unhandled node BoolOp(op=Or(), values=[Call(func=Attribute(value=Attribute(value=Name(id='ps', ctx=Load()), attr='poly', ctx=Load()), attr='get', ctx=Load()), args=[Constant(value='a')]), Constant(value=1)]) */
        a = _unk2r;
        double _unk3r = 0, _unk3i = 0;
        /* WARNING: unhandled node BoolOp(op=Or(), values=[Call(func=Attribute(value=Attribute(value=Name(id='ps', ctx=Load()), attr='poly', ctx=Load()), attr='get', ctx=Load()), args=[Constant(value='b')]), Constant(value=0)]) */
        b = _unk3r;
        double _unk4r = 0, _unk4i = 0;
        /* WARNING: unhandled node BoolOp(op=Or(), values=[Call(func=Attribute(value=Attribute(value=Name(id='ps', ctx=Load()), attr='poly', ctx=Load()), attr='get', ctx=Load()), args=[Constant(value='c')]), Constant(value=1)]) */
        c = _unk4r;
        double _call5r = 0, _call5i = 0;
        /* WARNING: unhandled call Attribute(value=Name(id='ps', ctx=Load()), attr='json2cvec', ctx=Load()) */
        cf_start = _call5r;
        double _np6r = 0, _np6i = 0;
        /* WARNING: unhandled np.poly */
        cf_end = _np6r;
    }
    double _call7r = 0, _call7i = 0;
    /* WARNING: unhandled call Attribute(value=Attribute(value=Name(id='np', ctx=Load()), attr='random', ctx=Load()), attr='uniform', ctx=Load()) */
    double _mul8r = 0, _mul8i = 0;
    c_mul(_call7r, _call7i, c, 0, &_mul8r, &_mul8i);
    double ta = _mul8r; /* +_mul8ii */
    double _call9r = 0, _call9i = 0;
    /* WARNING: unhandled call Attribute(value=Attribute(value=Name(id='np', ctx=Load()), attr='random', ctx=Load()), attr='uniform', ctx=Load()) */
    double _mul10r = 0, _mul10i = 0;
    c_mul(_call9r, _call9i, c, 0, &_mul10r, &_mul10i);
    double tb = _mul10r; /* +_mul10ii */
    double _c11r = 0, _c11i = 0;
    _c11r = 1.0; _c11i = 0;
    double _mul12r = 0, _mul12i = 0;
    c_mul(x1r, x1i, ta, 0, &_mul12r, &_mul12i);
    double _sub13r = 0, _sub13i = 0;
    _sub13r = _c11r - _mul12r; _sub13i = _c11i - _mul12i;
    double _attr14r = 0, _attr14i = 0;
    _attr14r = cf_start; _attr14i = 0;
    double _mul15r = 0, _mul15i = 0;
    c_mul(_sub13r, _sub13i, _attr14r, _attr14i, &_mul15r, &_mul15i);
    double _mul16r = 0, _mul16i = 0;
    c_mul(x1r, x1i, ta, 0, &_mul16r, &_mul16i);
    double _attr17r = 0, _attr17i = 0;
    _attr17r = cf_end; _attr17i = 0;
    double _mul18r = 0, _mul18i = 0;
    c_mul(_mul16r, _mul16i, _attr17r, _attr17i, &_mul18r, &_mul18i);
    double _add19r = 0, _add19i = 0;
    _add19r = _mul15r + _mul18r; _add19i = _mul15i + _mul18i;
    double real_part = _add19r; /* +_add19ii */
    double _c20r = 0, _c20i = 0;
    _c20r = 1.0; _c20i = 0;
    double _mul21r = 0, _mul21i = 0;
    c_mul(x2r, x2i, tb, 0, &_mul21r, &_mul21i);
    double _sub22r = 0, _sub22i = 0;
    _sub22r = _c20r - _mul21r; _sub22i = _c20i - _mul21i;
    double _attr23r = 0, _attr23i = 0;
    _attr23r = 0; _attr23i = 0;
    double _mul24r = 0, _mul24i = 0;
    c_mul(_sub22r, _sub22i, _attr23r, _attr23i, &_mul24r, &_mul24i);
    double _mul25r = 0, _mul25i = 0;
    c_mul(x2r, x2i, tb, 0, &_mul25r, &_mul25i);
    double _attr26r = 0, _attr26i = 0;
    _attr26r = 0; _attr26i = 0;
    double _mul27r = 0, _mul27i = 0;
    c_mul(_mul25r, _mul25i, _attr26r, _attr26i, &_mul27r, &_mul27i);
    double _add28r = 0, _add28i = 0;
    _add28r = _mul24r + _mul27r; _add28i = _mul24i + _mul27i;
    double imag_part = _add28r; /* +_add28ii */
    double _c29r = 0, _c29i = 0;
    _c29r = 0.0; _c29i = 1.0;
    double _mul30r = 0, _mul30i = 0;
    c_mul(_c29r, _c29i, imag_part, 0, &_mul30r, &_mul30i);
    double _add31r = 0, _add31i = 0;
    _add31r = real_part + _mul30r; _add31i = 0 + _mul30i;
    double coeffs = _add31r; /* +_add31ii */
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_rnd_path6_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double i = 0.0;
    double a = 0;
    double b = 0;
    double c = 0;
    double cf_end = 0;
    double cf_start = 0;
    double _c1r = 0, _c1i = 0;
    _c1r = 0.0; _c1i = 0;
    if (i == _c1r) {
        double _unk2r = 0, _unk2i = 0;
        /* WARNING: unhandled node BoolOp(op=Or(), values=[Call(func=Attribute(value=Attribute(value=Name(id='ps', ctx=Load()), attr='poly', ctx=Load()), attr='get', ctx=Load()), args=[Constant(value='a')]), Constant(value=1)]) */
        a = _unk2r;
        double _unk3r = 0, _unk3i = 0;
        /* WARNING: unhandled node BoolOp(op=Or(), values=[Call(func=Attribute(value=Attribute(value=Name(id='ps', ctx=Load()), attr='poly', ctx=Load()), attr='get', ctx=Load()), args=[Constant(value='b')]), Constant(value=0)]) */
        b = _unk3r;
        double _unk4r = 0, _unk4i = 0;
        /* WARNING: unhandled node BoolOp(op=Or(), values=[Call(func=Attribute(value=Attribute(value=Name(id='ps', ctx=Load()), attr='poly', ctx=Load()), attr='get', ctx=Load()), args=[Constant(value='c')]), Constant(value=1)]) */
        c = _unk4r;
        double _call5r = 0, _call5i = 0;
        /* WARNING: unhandled call Attribute(value=Name(id='ps', ctx=Load()), attr='json2cvec', ctx=Load()) */
        cf_start = _call5r;
        double _np6r = 0, _np6i = 0;
        /* WARNING: unhandled np.poly */
        cf_end = _np6r;
    }
    double _call7r = 0, _call7i = 0;
    /* WARNING: unhandled call Attribute(value=Attribute(value=Name(id='np', ctx=Load()), attr='random', ctx=Load()), attr='uniform', ctx=Load()) */
    double ta = _call7r; /* +_call7ii */
    double _call8r = 0, _call8i = 0;
    /* WARNING: unhandled call Attribute(value=Attribute(value=Name(id='np', ctx=Load()), attr='random', ctx=Load()), attr='uniform', ctx=Load()) */
    double tb = _call8r; /* +_call8ii */
    double _c9r = 0, _c9i = 0;
    _c9r = 1.0; _c9i = 0;
    double _sub10r = 0, _sub10i = 0;
    _sub10r = _c9r - x1r; _sub10i = _c9i - x1i;
    double _attr11r = 0, _attr11i = 0;
    _attr11r = cf_start; _attr11i = 0;
    double _mul12r = 0, _mul12i = 0;
    c_mul(_sub10r, _sub10i, _attr11r, _attr11i, &_mul12r, &_mul12i);
    double _attr13r = 0, _attr13i = 0;
    _attr13r = cf_end; _attr13i = 0;
    double _mul14r = 0, _mul14i = 0;
    c_mul(x1r, x1i, _attr13r, _attr13i, &_mul14r, &_mul14i);
    double _add15r = 0, _add15i = 0;
    _add15r = _mul12r + _mul14r; _add15i = _mul12i + _mul14i;
    double real_part = _add15r; /* +_add15ii */
    double _c16r = 0, _c16i = 0;
    _c16r = 1.0; _c16i = 0;
    double _sub17r = 0, _sub17i = 0;
    _sub17r = _c16r - x2r; _sub17i = _c16i - x2i;
    double _attr18r = 0, _attr18i = 0;
    _attr18r = 0; _attr18i = 0;
    double _mul19r = 0, _mul19i = 0;
    c_mul(_sub17r, _sub17i, _attr18r, _attr18i, &_mul19r, &_mul19i);
    double _attr20r = 0, _attr20i = 0;
    _attr20r = 0; _attr20i = 0;
    double _mul21r = 0, _mul21i = 0;
    c_mul(x2r, x2i, _attr20r, _attr20i, &_mul21r, &_mul21i);
    double _add22r = 0, _add22i = 0;
    _add22r = _mul19r + _mul21r; _add22i = _mul19i + _mul21i;
    double imag_part = _add22r; /* +_add22ii */
    double _c23r = 0, _c23i = 0;
    _c23r = 0.0; _c23i = 1.0;
    double _mul24r = 0, _mul24i = 0;
    c_mul(_c23r, _c23i, imag_part, 0, &_mul24r, &_mul24i);
    double _add25r = 0, _add25i = 0;
    _add25r = real_part + _mul24r; _add25i = 0 + _mul24i;
    double coeffs = _add25r; /* +_add25ii */
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}

static void poly_rnd_path7_c(double x1r, double x1i, double x2r, double x2i,
                     double *cRe, double *cIm, int *nCoeffs) {
    *nCoeffs = 36;
    for (int _i = 0; _i < 36; _i++) { cRe[_i] = 0; cIm[_i] = 0; }
    double i = 0.0;
    double a1 = 0;
    double a2 = 0;
    double b1 = 0;
    double b2 = 0;
    double cf_end = 0;
    double cf_start = 0;
    double n = 0;
    double _c1r = 0, _c1i = 0;
    _c1r = 0.0; _c1i = 0;
    if (i == _c1r) {
        double _unk2r = 0, _unk2i = 0;
        /* WARNING: unhandled node BoolOp(op=Or(), values=[Call(func=Attribute(value=Attribute(value=Name(id='ps', ctx=Load()), attr='poly', ctx=Load()), attr='get', ctx=Load()), args=[Constant(value='n')]), Constant(value=10)]) */
        n = _unk2r;
        double _unk3r = 0, _unk3i = 0;
        /* WARNING: unhandled node BoolOp(op=Or(), values=[Call(func=Attribute(value=Attribute(value=Name(id='ps', ctx=Load()), attr='poly', ctx=Load()), attr='get', ctx=Load()), args=[Constant(value='a1')]), Constant(value=1)]) */
        a1 = _unk3r;
        double _unk4r = 0, _unk4i = 0;
        /* WARNING: unhandled node BoolOp(op=Or(), values=[Call(func=Attribute(value=Attribute(value=Name(id='ps', ctx=Load()), attr='poly', ctx=Load()), attr='get', ctx=Load()), args=[Constant(value='a2')]), Constant(value=1)]) */
        a2 = _unk4r;
        double _unk5r = 0, _unk5i = 0;
        /* WARNING: unhandled node BoolOp(op=Or(), values=[Call(func=Attribute(value=Attribute(value=Name(id='ps', ctx=Load()), attr='poly', ctx=Load()), attr='get', ctx=Load()), args=[Constant(value='b1')]), Constant(value=0)]) */
        b1 = _unk5r;
        double _unk6r = 0, _unk6i = 0;
        /* WARNING: unhandled node BoolOp(op=Or(), values=[Call(func=Attribute(value=Attribute(value=Name(id='ps', ctx=Load()), attr='poly', ctx=Load()), attr='get', ctx=Load()), args=[Constant(value='b2')]), Constant(value=0)]) */
        b2 = _unk6r;
        double _np7r = 0, _np7i = 0;
        /* WARNING: unhandled np.poly */
        cf_start = _np7r;
        double _np8r = 0, _np8i = 0;
        /* WARNING: unhandled np.poly */
        cf_end = _np8r;
    }
    double _c9r = 0, _c9i = 0;
    _c9r = 1.0; _c9i = 0;
    double _sub10r = 0, _sub10i = 0;
    _sub10r = _c9r - x1r; _sub10i = _c9i - x1i;
    double _attr11r = 0, _attr11i = 0;
    _attr11r = cf_start; _attr11i = 0;
    double _mul12r = 0, _mul12i = 0;
    c_mul(_sub10r, _sub10i, _attr11r, _attr11i, &_mul12r, &_mul12i);
    double _attr13r = 0, _attr13i = 0;
    _attr13r = cf_end; _attr13i = 0;
    double _mul14r = 0, _mul14i = 0;
    c_mul(x1r, x1i, _attr13r, _attr13i, &_mul14r, &_mul14i);
    double _add15r = 0, _add15i = 0;
    _add15r = _mul12r + _mul14r; _add15i = _mul12i + _mul14i;
    double real_part = _add15r; /* +_add15ii */
    double _c16r = 0, _c16i = 0;
    _c16r = 1.0; _c16i = 0;
    double _sub17r = 0, _sub17i = 0;
    _sub17r = _c16r - x2r; _sub17i = _c16i - x2i;
    double _attr18r = 0, _attr18i = 0;
    _attr18r = 0; _attr18i = 0;
    double _mul19r = 0, _mul19i = 0;
    c_mul(_sub17r, _sub17i, _attr18r, _attr18i, &_mul19r, &_mul19i);
    double _attr20r = 0, _attr20i = 0;
    _attr20r = 0; _attr20i = 0;
    double _mul21r = 0, _mul21i = 0;
    c_mul(x2r, x2i, _attr20r, _attr20i, &_mul21r, &_mul21i);
    double _add22r = 0, _add22i = 0;
    _add22r = _mul19r + _mul21r; _add22i = _mul19i + _mul21i;
    double imag_part = _add22r; /* +_add22ii */
    double _c23r = 0, _c23i = 0;
    _c23r = 0.0; _c23i = 1.0;
    double _mul24r = 0, _mul24i = 0;
    c_mul(_c23r, _c23i, imag_part, 0, &_mul24r, &_mul24i);
    double _add25r = 0, _add25i = 0;
    _add25r = real_part + _mul24r; _add25i = 0 + _mul24i;
    double coeffs = _add25r; /* +_add25ii */
    for (int _i = 0; _i < 36; _i++) {
        if (!isfinite(cRe[_i]) || !isfinite(cIm[_i])) { cRe[_i] = 0; cIm[_i] = 0; }
    }
}
