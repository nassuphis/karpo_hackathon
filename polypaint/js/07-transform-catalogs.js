// PolyPaint 07-transform-catalogs — split from index.html's single script block.
// Classic script: load order matters and is defined by the
// <script src="js/..."> tags in index.html (top-level functions and
// let/const bindings are shared across all parts, exactly as before
// the split). Cache-busting: deploy appends ?v=<BUILD_ID> to the tags.
const _ptCategoryMeta = {
    maps: { title: 'Maps', help: 'direct complex maps applied to t1/t2' },
    arithmetic: { title: 'Arithmetic', help: 'offset, scale, invert, or remix parameter values' },
    shapes: { title: 'Curves + shapes', help: 'replace selected parameters with points on analytic curves' },
    roots: { title: 'Root-derived', help: 'derive t1/t2 from roots of small polynomials' },
    dither: { title: 'Dither', help: 'jitter parameters before coefficient generation' },
    legacy: { title: 'Legacy coefficient maps', help: 'older named parameter recipes kept for compatibility' },
};

const _ptInfo = {
    unit_circle: { category: 'maps', desc: 'map real t to exp(2*pi*i*t)' },
    rtheta: { category: 'maps', desc: 'polar disk map using t1/t2 as radius and angle' },
    square: { category: 'maps', desc: 'square both complex parameters' },
    cube: { category: 'maps', desc: 'cube both complex parameters' },
    reciprocal: { category: 'maps', desc: 'replace each parameter with 1/t' },
    conjugate: { category: 'maps', desc: 'complex conjugate t1 and t2' },
    swap: { category: 'maps', desc: 'swap t1 and t2' },
    add_sub: { category: 'arithmetic', desc: 'emit t1+t2 and t1-t2' },
    mul_div: { category: 'arithmetic', desc: 'emit t1*t2 and t1/t2' },
    moebius: { category: 'maps', desc: 'Mobius map on t1 and t2; 0-arg runs keep legacy 1/(t+2)' },
    negate: { category: 'maps', desc: 'negate t1 and t2' },
    exp: { category: 'maps', desc: 'complex exponential of each parameter' },
    xim: { category: 'maps', desc: 'move real part into the imaginary axis' },
    zzold: { category: 'arithmetic', desc: 'legacy t1+i*t2 remix into both slots' },
    zz1: { category: 'arithmetic', desc: 't1+i*t2 and t1*t2+i*(t1+t2)' },
    zz2: { category: 'arithmetic', desc: 't1+i*t2 and t1-i*t2' },
    zz3: { category: 'arithmetic', desc: 'cross-imaginary remix of t1 and t2' },
    inv_t_plus_2: { category: 'arithmetic', desc: 't1=1/(t1+a), t2=1/(t2+b)' },
    t1radd: { category: 'arithmetic', desc: 'add to real part of t1 only' },
    t1iadd: { category: 'arithmetic', desc: 'add to imaginary part of t1 only' },
    t2radd: { category: 'arithmetic', desc: 'add to real part of t2 only' },
    t2iadd: { category: 'arithmetic', desc: 'add to imaginary part of t2 only' },
    radd: { category: 'arithmetic', desc: 'add to real parts of both parameters' },
    iadd: { category: 'arithmetic', desc: 'add to imaginary parts of both parameters' },
    add: { category: 'arithmetic', desc: 'z1=z1+c1 and z2=z2+c2 using complex offsets' },
    cadd: { category: 'arithmetic', desc: 'add a complex constant to t1 and t2' },
    rscale: { category: 'arithmetic', desc: 'scale real parts only' },
    iscale: { category: 'arithmetic', desc: 'scale imaginary parts only' },
    scale: { category: 'arithmetic', desc: 'scale all components' },
    crd: { category: 'shapes', desc: 'cardioid curve' },
    hrt: { category: 'shapes', desc: 'heart curve' },
    spdl: { category: 'shapes', desc: 'spindle curve' },
    lmc: { category: 'shapes', desc: 'limacon curve' },
    rsc: { category: 'shapes', desc: 'rose curve' },
    lss: { category: 'shapes', desc: 'Lissajous curve' },
    ast: { category: 'shapes', desc: 'astroid curve' },
    asp: { category: 'shapes', desc: 'Archimedean spiral' },
    lsp: { category: 'shapes', desc: 'log spiral' },
    dlt: { category: 'shapes', desc: 'deltoid curve' },
    rply: { category: 'shapes', desc: 'regular polygon perimeter' },
    star: { category: 'shapes', desc: 'star polygon perimeter' },
    rect: { category: 'shapes', desc: 'rectangle perimeter' },
    rrect: { category: 'shapes', desc: 'rounded rectangle / superellipse' },
    z01: { category: 'arithmetic', desc: 'real-part mirror remix' },
    sum_prod: { category: 'arithmetic', desc: 'emit t1+t2 and t1*t2' },
    roots2: { category: 'roots', desc: 'quadratic roots of (9/64)z^2+t1*z+t2' },
    roots3: { category: 'roots', desc: 'cubic roots from t1+t2, 1, 1, t1*t2' },
    roots5: { category: 'roots', desc: 'cubic roots from trig/i*t parameters' },
    roots6: { category: 'roots', desc: 'quartic roots from mixed t1/t2 polynomial' },
    sdith: { category: 'dither', desc: 'square uniform jitter' },
    ddith: { category: 'dither', desc: 'disk jitter' },
    adth: { category: 'dither', desc: 'annulus jitter' },
    ldth: { category: 'dither', desc: 'line-segment jitter' },
    crdth: { category: 'dither', desc: 'cross-shaped jitter' },
    scdth: { category: 'dither', desc: 'sector jitter' },
    ndith: { category: 'dither', desc: 'normal/Gaussian jitter' },
    coeff2: { category: 'legacy', desc: 'legacy t1+t2 and t1*t2 map' },
    coeff3: { category: 'legacy', desc: 'legacy 1/(t+2) map' },
    coeff3a: { category: 'legacy', desc: 'legacy 1/(t+1) map' },
    coeff4: { category: 'legacy', desc: 'legacy cos(t1), sin(t2)' },
    coeff5: { category: 'legacy', desc: 'legacy reciprocal cross map' },
    coeff5a: { category: 'legacy', desc: 'legacy reciprocal self map' },
    coeff6: { category: 'legacy', desc: 'legacy cubic fractional map' },
    coeff7: { category: 'legacy', desc: 'legacy trig fractional map' },
    coeff8: { category: 'legacy', desc: 'legacy cross trig map' },
    coeff9: { category: 'legacy', desc: 'legacy squared fractional map' },
    coeff10: { category: 'legacy', desc: 'legacy fourth-power fractional map' },
    coeff11: { category: 'legacy', desc: 'legacy log fourth-power map' },
    coeff12: { category: 'legacy', desc: 'legacy mixed polynomial map' },
};

// Declaration form: top-level executable statements are reserved for the
// js/12 boot block (parts-contract test); this enrichment must run here at
// load because later parts read the enriched catalog while parsing.
const _ptCatalogEnriched = Object.entries(_ptInfo).every(([name, info]) => {
    if (_ptCatalog[name]) Object.assign(_ptCatalog[name], info);
    return true;
});

const _paramProgramLegacyNames = [
    'none',
    'unit_circle',
    'square',
    'cube',
    'reciprocal',
    'conjugate',
    'negate',
    'exp',
    'xim',
    'add_sub',
    'mul_div',
    'swap',
    'sum_prod',
    'roots2',
    'roots3',
    'roots5',
    'roots6',
    'rtheta',
    'moebius',
    'inv_t_plus_2',
    'crd',
    'hrt',
    'spdl',
    'lmc',
    'rsc',
    'lss',
    'ast',
    'asp',
    'lsp',
    'dlt',
    'rply',
    'star',
    'rect',
    'rrect',
    'z01',
    'coeff2',
    'coeff3',
    'coeff3a',
    'coeff4',
    'coeff5',
    'coeff5a',
    'coeff6',
    'coeff7',
    'coeff8',
    'coeff9',
    'coeff10',
    'coeff11',
    'coeff12',
];

const _paramProgramIndependentLegacyTargets = new Set([
    'unit_circle',
    'square',
    'cube',
    'reciprocal',
    'conjugate',
    'negate',
    'exp',
    'xim',
]);

const _paramProgramLegacyTargetArgIndexes = {
    rtheta: 1,
    crd: 0,
    hrt: 0,
    spdl: 0,
    lmc: 0,
    rsc: 0,
    lss: 0,
    ast: 0,
    asp: 0,
    lsp: 0,
    dlt: 0,
    rply: 0,
    star: 0,
    rect: 0,
    rrect: 0,
};

const _paramProgramLegacyArgSpecs = {
    rtheta: [{ ph: 'p', def: '1', scalarExpr: true, exprWide: true, title: 'Real expression. Registers: t1, t2, p1, p2. Constants: pi, pi2, pi2i. Use real(...), imag(...), abs(...), or mod(...) for complex registers.' }],
    moebius: [
        { ph: 'a', def: '1', scalarExpr: true, complexWide: true, title: 'Complex expression. Registers: t1, t2, p1, p2. Constants: pi, pi2, pi2i. Functions: exp, real, imag, abs, mod.' },
        { ph: 'b', def: '0', scalarExpr: true, complexWide: true, title: 'Complex expression. Registers: t1, t2, p1, p2. Constants: pi, pi2, pi2i. Functions: exp, real, imag, abs, mod.' },
        { ph: 'c', def: '0', scalarExpr: true, complexWide: true, title: 'Complex expression. Registers: t1, t2, p1, p2. Constants: pi, pi2, pi2i. Functions: exp, real, imag, abs, mod.' },
        { ph: 'd', def: '1', scalarExpr: true, complexWide: true, title: 'Complex expression. Registers: t1, t2, p1, p2. Constants: pi, pi2, pi2i. Functions: exp, real, imag, abs, mod.' },
    ],
    inv_t_plus_2: [
        { ph: 'z1', def: '2', scalarExpr: true, complexWide: true, title: 'Complex expression for the first offset.' },
        { ph: 'z2', def: '2', scalarExpr: true, complexWide: true, title: 'Complex expression for the second offset.' },
    ],
    crd: [{ ph: 'size', def: '1', scalarExpr: true, exprWide: true }],
    hrt: [{ ph: 'size', def: '1', scalarExpr: true, exprWide: true }, { ph: 'turns', def: '0', scalarExpr: true, exprWide: true }],
    spdl: [{ ph: 'va', def: '0.5', scalarExpr: true, exprWide: true }, { ph: 'vb', def: '0.2', scalarExpr: true, exprWide: true }, { ph: 'vp', def: '1.5', scalarExpr: true, exprWide: true }],
    lmc: [{ ph: 'a', def: '0.3', scalarExpr: true, exprWide: true }, { ph: 'b', def: '0.5', scalarExpr: true, exprWide: true }],
    rsc: [{ ph: 'amp', def: '0.5', scalarExpr: true, exprWide: true }, { ph: 'k', def: '2', scalarExpr: true, exprWide: true }],
    lss: [{ ph: 'A', def: '0.5', scalarExpr: true, exprWide: true }, { ph: 'B', def: '0.5', scalarExpr: true, exprWide: true }, { ph: 'a', def: '3', scalarExpr: true, exprWide: true }, { ph: 'b', def: '2', scalarExpr: true, exprWide: true }, { ph: 'phase', def: '0.5', scalarExpr: true, exprWide: true }],
    ast: [{ ph: 'scale', def: '1', scalarExpr: true, exprWide: true }],
    asp: [{ ph: 'a', def: '0', scalarExpr: true, exprWide: true }, { ph: 'b', def: '0.1', scalarExpr: true, exprWide: true }],
    lsp: [{ ph: 'a', def: '0.1', scalarExpr: true, exprWide: true }, { ph: 'b', def: '0.15', scalarExpr: true, exprWide: true }],
    dlt: [{ ph: 'R', def: '1', scalarExpr: true, exprWide: true }],
    rply: [{ ph: 'sides', def: '5', scalarExpr: true, exprWide: true }, { ph: 'radius', def: '1', scalarExpr: true, exprWide: true }, { ph: 'turns', def: '0', scalarExpr: true, exprWide: true }],
    star: [{ ph: 'points', def: '5', scalarExpr: true, exprWide: true }, { ph: 'outer', def: '1', scalarExpr: true, exprWide: true }, { ph: 'inner', def: '0.5', scalarExpr: true, exprWide: true }],
    rect: [{ ph: 'width', def: '2', scalarExpr: true, exprWide: true }, { ph: 'height', def: '1', scalarExpr: true, exprWide: true }, { ph: 'turns', def: '0', scalarExpr: true, exprWide: true }],
    rrect: [{ ph: 'width', def: '2', scalarExpr: true, exprWide: true }, { ph: 'height', def: '1', scalarExpr: true, exprWide: true }, { ph: 'm', def: '4', scalarExpr: true, exprWide: true }],
};

const _ppCategoryMeta = {
    io: { title: 'Input + output', help: 'read t1/t2 or write p1/p2 registers' },
    stack: { title: 'Stack', help: 'reorder or discard temporary complex values' },
    arithmetic: { title: 'Arithmetic', help: 'combine complex stack values' },
    unary: { title: 'Complex maps', help: 'apply one complex transform to top of stack' },
    bridge: { title: 'Legacy bridge', help: 'call existing parameter transforms by integer function index after compile' },
};

const _ppCatalog = {
    push: {
        category: 'io',
        params: [{ ph: 'src', def: 't1', choices: ['t1', 't2', 'both'], selectorWide: true }],
        desc: 'push t1, t2, or both onto the stack',
    },
    emit: {
        category: 'io',
        params: [{ ph: 'target', def: 'p1', choices: ['p1', 'p2'], selectorWide: true }],
        desc: 'pop one stack value into output register p1 or p2',
    },
    const: {
        category: 'io',
        params: [{ ph: 'value', def: '0', scalarExpr: true, complexWide: true, title: 'Complex expression. Registers: t1, t2, p1, p2. Constants: pi, pi2, pi2i. Functions: exp, real, imag, abs, mod.' }],
        desc: 'push a complex expression value',
    },
    macro: {
        category: 'bridge',
        params: [{ ph: 'program id', def: '', programWide: true, title: 'Saved param-program id to expand at compile time.' }],
        desc: 'expand a saved param program at compile time',
    },
    duplicate: { category: 'stack', desc: 'duplicate the top stack value' },
    swap: { category: 'stack', desc: 'swap the top two stack values' },
    pop: { category: 'stack', desc: 'discard the top stack value' },
    flush: { category: 'stack', desc: 'clear the whole temporary stack' },
    add: { category: 'arithmetic', desc: 'a+b; top of stack is b' },
    subtract: { category: 'arithmetic', desc: 'a-b; top of stack is b' },
    mul: { category: 'arithmetic', desc: 'a*b' },
    ratio: { category: 'arithmetic', desc: 'a/b; zero when denominator is zero' },
    negate: { category: 'unary', desc: '-z' },
    conj: { category: 'unary', desc: 'complex conjugate' },
    reciprocal: { category: 'unary', desc: '1/z' },
    unit_circle: { category: 'unary', desc: 'exp(2*pi*i*z.re) with imaginary scaling' },
    square: { category: 'unary', desc: 'z squared' },
    cube: { category: 'unary', desc: 'z cubed' },
    exp: { category: 'unary', desc: 'complex exponential' },
    coeff2: { category: 'bridge', desc: 'legacy t1+t2 and t1*t2 map' },
    coeff3: { category: 'bridge', desc: 'legacy 1/(t+2) map' },
    coeff3a: { category: 'bridge', desc: 'legacy 1/(t+1) map' },
    coeff4: { category: 'bridge', desc: 'legacy cos(t1), sin(t2)' },
    coeff5: { category: 'bridge', desc: 'legacy reciprocal cross map' },
    coeff5a: { category: 'bridge', desc: 'legacy reciprocal self map' },
    coeff6: { category: 'bridge', desc: 'legacy cubic fractional map' },
    coeff7: { category: 'bridge', desc: 'legacy trig fractional map' },
    coeff8: { category: 'bridge', desc: 'legacy cross trig map' },
    coeff9: { category: 'bridge', desc: 'legacy squared fractional map' },
    coeff10: { category: 'bridge', desc: 'legacy fourth-power fractional map' },
    coeff11: { category: 'bridge', desc: 'legacy log fourth-power map' },
    coeff12: { category: 'bridge', desc: 'legacy mixed polynomial map' },
    legacy: {
        category: 'bridge',
        params: [
            { ph: 'name', def: 'unit_circle', choices: _paramProgramLegacyNames, paramProgramWide: true },
            { ph: 'src', def: 'both', choices: ['p1', 'p2', 'both', 'pop1', 'pop2'], selectorWide: true },
            { ph: 'tgt', def: 'both', choices: ['p1', 'p2', 'both', 'push1', 'push2'], selectorWide: true },
            { ph: 'args', def: '', scalarExpr: true, complexWide: true },
        ],
        desc: 'call an existing param transform with explicit source and target selectors',
    },
};

const _rtCatalog = {
    rotate_roots:     { params: [{ph:'turns', def:'0'}] },
    pull_unit_circle: { params: [{ph:'sigma', def:'0.75'}, {ph:'alpha', def:'1'}] },
    roots_toline:     {},
    line_to_unit_circle: {},
    invert_roots:     { label: '1/z' },
    add_complex:      { params: [{ph:'a', def:'0'}, {ph:'b', def:'0'}], label: 'z+(a+ib)' },
    mul_complex:      { params: [{ph:'a', def:'1'}, {ph:'b', def:'0'}], label: 'z*(a+ib)' },
    moebius:          { params: [{ph:'a', def:'1'}, {ph:'b', def:'0'}, {ph:'c', def:'0'}, {ph:'d', def:'1'}], label: 'moebius' },
    pull_towards_center: { params: [{ph:'alpha', def:'1'}, {ph:'sigma', def:'0.75'}] },
};

const _solveScoreMetricNames = [
    'proximity',
    'crowding',
    'spread',
    'anisotropy',
    'area',
    'clusteriness',
    'shelliness',
    'outlierness',
    'nn_variation',
    'real_axis_proximity',
    'centroid_re',
    'centroid_im',
    'centroid_dist',
    'dist_unit_circle',
    'asymmetry_re',
    'max_re',
    'min_re',
    'max_im',
    'min_im',
    'min_mod',
    'max_mod',
    'min_angular_separation',
    'mean_log_mod',
    'sd_log_mod',
    'inside_unit_fraction',
    'unit_annulus_fraction_01',
    'imag_axis_proximity',
    'diagonal_proximity',
    'angular_entropy_16',
    'sector_max_share_16',
    'angular_order_2',
    'angular_order_3',
    'angular_order_4',
    't1_re',
    't1_im',
    't1_abs',
    't1_phase',
    't2_re',
    't2_im',
    't2_abs',
    't2_phase',
];
const _solveScoreParamMetricNames = [
    't1_re',
    't1_im',
    't1_abs',
    't1_phase',
    't2_re',
    't2_im',
    't2_abs',
    't2_phase',
];
const _solveScoreParamCapableMetricNames = [
    'max_re',
    'min_re',
    'max_im',
    'min_im',
    'min_mod',
    'max_mod',
];
const _solveScoreMetricSet = new Set(_solveScoreMetricNames);
const _solveScoreParamMetricSet = new Set(_solveScoreParamMetricNames);
const _solveScoreParamCapableMetricSet = new Set(_solveScoreParamCapableMetricNames);
const _solveScoreGenericMetricPublicName = 'metric';
const _solveScoreGenericMetricChipName = '__metric';
const _solveScoreGenericMetricNames = _solveScoreMetricNames.filter(name => {
    const sources = _solveScoreMetricAllowedSources(name);
    return sources.includes('slv') && sources.includes('cf');
});
const _solveScoreGenericSourceChoices = ['slv', 'cf', 'slv-1', 'cf-1'];
const _solveScoreSourceChoices = ['slv', 'cf', 'pm'];
const _solveScoreUnarySpecs = {
    omega_cosine: { arity: 1, params: [{ ph: 'w', def: '1' }, { ph: 'phase', def: '0' }], tooltip: 'unary stack op: g(u)=0.5*(cos(omega*2*pi*u+phase)+1)' },
    sawtooth: { arity: 1, params: [{ ph: 'mult', def: '10' }], tooltip: 'unary stack op: frac(score*mult)' },
    flip: { arity: 1, params: [], tooltip: 'unary stack op: 1-score' },
    const: { arity: 0, params: [{ ph: 'value', def: '0' }], tooltip: 'push a finite constant onto the stack' },
    dup: { arity: 1, params: [], tooltip: 'duplicate the top stack value' },
    flush: { arity: 0, params: [], tooltip: 'clear the entire score stack' },
    clamp: { arity: 1, params: [], tooltip: 'clamp the top stack value to [0,1]' },
    sin: { arity: 1, params: [], tooltip: 'raw sin(score), radians' },
    cos: { arity: 1, params: [], tooltip: 'raw cos(score), radians' },
    log: { arity: 1, params: [], tooltip: 'natural log; invalid inputs become 0' },
    exp: { arity: 1, params: [], tooltip: 'raw exp(score); overflow becomes 0' },
    pow: { arity: 1, params: [{ ph: 'pow', def: '2' }], tooltip: 'raw pow(score, exponent)' },
};
const _solveScoreOutputSpecs = {
    emit: { arity: 1, params: [{ ph: 'mode', def: 'norm', choices: ['raw', 'norm', 'none'] }], tooltip: 'pop one score; raw/norm emit a byte, none discards it for debugging branches' },
    emit_norm: { arity: 1, params: [], tooltip: 'legacy alias for emit(norm)' },
    emit_none: { arity: 1, params: [], tooltip: 'legacy alias for emit(none)' },
};
const _solveScoreCombineSpecs = {
    avg: { arity: 2, params: [] },
    min: { arity: 2, params: [] },
    max: { arity: 2, params: [] },
    mul: { arity: 2, params: [] },
    add: { arity: 2, params: [], tooltip: 'raw a+b' },
    mult: { arity: 2, params: [], tooltip: 'raw a*b' },
    subtract: { arity: 2, params: [], tooltip: 'raw a-b; top of stack is b' },
    ratio: { arity: 2, params: [], tooltip: 'raw a/b; zero if denominator is zero' },
    ema: { arity: 2, params: [{ ph: 'alpha', def: '0.99' }], tooltip: 'a*alpha + b*(1-alpha), b is top/latest' },
    weighted_sum: { arity: 2, params: [{ ph: 'a', def: '0.5' }, { ph: 'b', def: '0.5' }] },
    abs_diff: { arity: 2, params: [] },
    geometric_mean: { arity: 2, params: [] },
};
const _ssCatalog = (() => {
    const catalog = {};
    catalog[_solveScoreGenericMetricChipName] = {
        label: 'metric',
        chip_kind: 'metric',
        params: [
            { ph: 'metric', def: _solveScoreGenericMetricNames[0] || 'proximity', choices: _solveScoreGenericMetricNames },
            { ph: 'src', def: 'slv', choices: _solveScoreGenericSourceChoices },
            { ph: 'q%', def: '0.1' },
        ],
        tooltip: 'generic metric chip: choose score metric, slv/cf source, and q%',
    };
    _solveScoreMetricNames.forEach(name => {
        catalog[name] = {
            label: name,
            chip_kind: 'metric',
            params: [{ ph: 'src', def: _solveScoreParamMetricSet.has(name) ? 'pm' : 'slv', choices: _solveScoreMetricSourceChoices(name) }, { ph: 'q%', def: '0.1' }],
            tooltip: _solveScoreParamMetricSet.has(name) ? `${name}(pm,q=0.1%)` : `${name}(slv,q=0.1%)`,
        };
    });
    Object.entries(_solveScoreCombineSpecs).forEach(([name, spec]) => {
        catalog[name] = {
            label: name,
            chip_kind: 'combine',
            params: spec.params || [],
            tooltip: `stack ${spec.arity} -> 1`,
        };
    });
    Object.entries(_solveScoreUnarySpecs).forEach(([name, spec]) => {
        catalog[name] = {
            label: name,
            chip_kind: 'unary',
            params: spec.params || [],
            tooltip: spec.tooltip || `stack ${spec.arity} -> ${spec.arity}`,
        };
    });
    Object.entries(_solveScoreOutputSpecs).forEach(([name, spec]) => {
        catalog[name] = {
            label: name,
            chip_kind: 'output',
            params: spec.params || [],
            tooltip: spec.tooltip || 'stack 1 -> output',
        };
    });
    return catalog;
})();

const _coeffProgramScalarExprHelp = 'Program mode accepts t1/t2, p1/p2, poly_len, cfN, polyN, tosN, pi, pi2, pi2i, literals, + - * /, and conj/real/imag/abs/angle/sqrt/log/exp/sin/cos/tan/sinh/cosh/tanh.';
// kind: 'andy' is the semantic marker (filtering/serialization/validation
// key off it); ph is display-only placeholder text.
const _ctAndyParam = { kind: 'andy', ph: 'andy', label: 'andy', def: '0', scalarExpr: 'real', title: `Blend amount in [0,1]. ${_coeffProgramScalarExprHelp} In Chain mode it must be numeric.` };
function _isAndyParam(pDef) {
    return !!(pDef && pDef.kind === 'andy');
}
// Registry-transform vocabulary, generated from lambda/coeff_legacy_registry.json
// (lambda/gen_coeff_vocab.py -> coeff_vocab_js.js, regenerated on every deploy
// and loaded as a script tag before this bundle). Single source of truth with
// the backend compilers for aliases (wire format: saved chip rows carry
// them), chip-name shadowing, and text-alias synthesis.
const _coeffRegistryVocab = (typeof window !== 'undefined' && window._coeffRegistryVocab) || null;
// (top-level statement moved to the js/12 boot block — parts are
//  declarations-only; see tests/test_frontend_parts_contract.py)

// Adding a coeff transform end to end:
//   1. lambda/coeff_legacy_registry.json — new entry with the next fn_index
//      (wire format: never renumber; tests/test_coeff_program_drift.py pins
//      them) plus its ui block (label/desc/params — this is where the chip's
//      fields and tooltips live now).
//   2. lambda/sweep_cli.c — implement ct_<name> and add its case to the legacy
//      fn_index switch; rebuild sweep_test (cc -O2 -pthread) and the ARM binaries.
//   3. lambda/gen_coeff_vocab.py regenerates coeff_vocab_js.js (deploy does
//      this automatically; predeploy --check fails on a stale file).
//   4. tests/test_coeff_program_native.py — a native value test for the new fn.
//
// _ctCatalog hydrates from the generated registry vocab: titles carry a
// {SCALAR_EXPR_HELP} placeholder resolved here, and every transform gets the
// shared andy param appended (all registry transforms support andy).
function _hydrateCtParamDef(pDef) {
    const out = { ...pDef };
    if (out.title) out.title = out.title.replace('{SCALAR_EXPR_HELP}', _coeffProgramScalarExprHelp);
    return out;
}
const _ctCatalog = (() => {
    const catalog = {};
    const specs = _coeffRegistryVocab ? _coeffRegistryVocab.ctCatalog : {};
    Object.entries(specs).forEach(([name, spec]) => {
        const entry = {
            category: spec.category,
            desc: spec.desc,
            params: [...(spec.params || []).map(_hydrateCtParamDef), { ..._ctAndyParam }],
        };
        if (spec.label) entry.label = spec.label;
        catalog[name] = entry;
    });
    return catalog;
})();

const _ctCategoryMeta = _coeffRegistryVocab ? _coeffRegistryVocab.categoryMeta : {};
function _coeffProgramWideParamDefs(name) {
    // Program-mode wide-editor defs for exp/round legacy chips (different
    // defaults and titles than the chain-row fields), from the registry ui.
    const defs = (_coeffRegistryVocab && _coeffRegistryVocab.programParamDefs[name]) || [];
    return [...defs.map(_hydrateCtParamDef), { ..._ctAndyParam }];
}
const _coeffProgramExpParamDefs = _coeffProgramWideParamDefs('exp');
const _coeffProgramRoundParamDefs = _coeffProgramWideParamDefs('round');
const _coeffProgramCategoryMeta = {
    io: { title: 'Input + output', help: 'push cf/poly, make constant vectors, or emit poly' },
    stack: { title: 'Stack', help: 'reorder or discard vector values' },
    vector: { title: 'Vector ops', help: 'combine vectors on the stack' },
    structural: { title: 'Structural transforms', help: 'native transforms that reorder, resize, sanitize, or normalize poly vectors' },
    accumulation: { title: 'Ordering + accumulation', help: 'native transforms that sort or accumulate coefficient vectors' },
    elementwise: { title: 'Elementwise transforms', help: 'native transforms that apply a complex function per coefficient' },
    roots: { title: 'Root transforms', help: 'native transforms that replace coefficients with root-derived vectors' },
    macro: { title: 'Macro', help: 'expand a saved coeff program at compile time' },
};

const _coeffProgramLegacyNames = Object.keys(_ctCatalog);
function _paramValueOrDefault(params, idx, pDef) {
    return params[idx] !== undefined && params[idx] !== ''
        ? params[idx]
        : String((pDef && pDef.def) || '');
}

function _chainDisplayString(chain) {
    return chain.map(row => Array.isArray(row)
        ? (row.length > 1 ? `${row[0]}(${row.slice(1).join(',')})` : row[0])
        : String(row)).join('; ');
}

function _str(value) {
    return String(value == null ? '' : value);
}

function _pluralize(count, word) {
    return `${count} ${word}${count === 1 ? '' : 's'}`;
}

const _coeffProgramVectorBinaryNames = ['add', 'subtract', 'multiply', 'divide', 'power'];
const _coeffProgramVectorUnaryNames = ['angle', 'mod', 'abs', 'neg', 'conj', 'sqrt', 'log', 'exp', 'sin', 'cos', 'tan', 'sinh', 'cosh', 'tanh'];
const _coeffProgramVectorSourceChoices = ['poly', 'pop', 'peek'];
const _coeffProgramLegacySourceChoices = ['cf', 'poly', 'pop', 'peek'];
const _coeffProgramTargetChoices = ['poly', 'push'];
function _canonicalCoeffTransformName(name) {
    // Aliases come from the registry vocab above, so imported legacy rows
    // resolve the same _ctCatalog metadata the backend compiles them against.
    const raw = String(name || '').trim();
    const aliases = _coeffRegistryVocab ? _coeffRegistryVocab.aliasToCanonical : {};
    return aliases[raw] || raw;
}
function _coeffProgramLegacyInputDefs(legacyName) {
    const canonicalName = _canonicalCoeffTransformName(legacyName);
    if (canonicalName === 'exp') return _coeffProgramExpParamDefs.map(p => ({ ...p }));
    if (canonicalName === 'round') return _coeffProgramRoundParamDefs.map(p => ({ ...p }));
    const spec = _ctCatalog[canonicalName] || {};
    return (spec.params || []).map(p => ({ ...p }));
}

const _coeffProgramCatalog = (() => {
    const catalog = {
        push: {
            category: 'io',
            params: [{ ph: 'src', def: 'cf', choices: ['cf', 'poly'], paramProgramWide: true }],
            desc: 'push a copy of cf or current poly',
        },
        set: {
            category: 'io',
            params: [
                { ph: 'tgt', def: 'poly', choices: ['poly'], selectorWide: true, title: 'Output target. v1 supports writing poly.' },
                { ph: 'src', def: 'pop', choices: _coeffProgramLegacySourceChoices, selectorWide: true, title: 'Input vector to copy into poly.' },
            ],
            desc: 'copy a vector source into poly',
        },
        affine: {
            category: 'vector',
            label: 'linear',
            // hidden: parser-internal strict form; users author `linear`,
            // which serializes through this chip's params.
            hidden: true,
            params: [
                { ph: 'tgt', def: 'poly', choices: _coeffProgramTargetChoices, selectorWide: true, title: 'Output target: write poly or push the affine result.' },
                { ph: 'src', def: 'pop', choices: _coeffProgramLegacySourceChoices, selectorWide: true, title: 'Input vector.' },
                { ph: 'multiplier expr', label: 'a', def: '1', scalarExpr: 'complex', complexWide: true, title: `Complex multiplier expression. ${_coeffProgramScalarExprHelp}` },
                { ph: 'offset expr', label: 'b', def: '0', scalarExpr: 'complex', complexWide: true, title: `Complex offset expression. ${_coeffProgramScalarExprHelp}` },
            ],
            desc: 'apply z*a+b with complex expression arguments',
        },
        push_vec: {
            category: 'io',
            label: 'push_vec',
            params: [
                { ph: 'length|poly_len', label: 'length', def: 'poly_len', lengthArg: true, title: 'Vector length. Use poly_len to match the current poly vector, or an integer 1..256.' },
                { ph: 'value expr', label: 'value', def: '1', scalarExpr: 'complex', title: `Scalar expression for every coefficient. ${_coeffProgramScalarExprHelp}` },
            ],
            desc: 'push a constant vector push_vec(length, value); value is a scalar expression',
        },
        push_scalar: {
            category: 'stack',
            label: 'push_scalar',
            // hidden: emitted by the text-source lowerer; not user-addable.
            hidden: true,
            params: [
                { ph: 'value expr', label: 'value', def: '1', scalarExpr: 'complex', complexWide: true, title: `Scalar expression pushed as one scalar stack item. ${_coeffProgramScalarExprHelp}` },
            ],
            desc: 'push one scalar stack item from a scalar expression',
        },
        push_const: {
            category: 'io',
            label: 'push_const',
            hidden: true,
            params: [
                { ph: 'length|poly_len', label: 'length', def: 'poly_len', lengthArg: true, title: 'Vector length. Use poly_len to match the current poly vector, or an integer 1..256.' },
                { ph: 'value expr', label: 'value', def: '1', scalarExpr: 'complex', title: `Scalar expression for every coefficient. ${_coeffProgramScalarExprHelp}` },
            ],
            desc: 'back-compat alias for push_vec(length, value)',
        },
        push_linspace: {
            category: 'io',
            params: [
                { ph: 'length|poly_len', label: 'length', def: 'poly_len', lengthArg: true, title: 'Vector length. Use poly_len to match the current poly vector, or an integer 1..256.' },
            ],
            desc: 'push linspace values from 0 to length using length entries',
        },
        push_range: {
            category: 'io',
            params: [
                { ph: 'length|poly_len', label: 'length', def: 'poly_len', lengthArg: true, title: 'Vector length. Use poly_len to match the current poly vector, or an integer 1..256.' },
            ],
            desc: 'push Python-style range values 0, 1, ..., length-1',
        },
        poke_poly: {
            category: 'io',
            params: [
                { ph: 'index', label: 'index', def: '0', intLiteral: true, min: 0, max: 255, title: 'Zero-based poly coefficient index. The current poly vector must contain this index at runtime.' },
                { ph: 'value expr', label: 'value', def: '100j*p1', scalarExpr: 'complex', title: `Scalar expression written into poly[index]. ${_coeffProgramScalarExprHelp}` },
            ],
            desc: 'write value into poly[index] without touching the stack',
        },
        poke_tos: {
            category: 'stack',
            params: [
                { ph: 'index', label: 'index', def: '0', intLiteral: true, min: 0, max: 255, title: 'Zero-based index in the top-of-stack vector. Requires a stack value at runtime.' },
                { ph: 'value expr', label: 'value', def: '100j*p1', scalarExpr: 'complex', title: `Scalar expression written into top_of_stack[index]. ${_coeffProgramScalarExprHelp}` },
            ],
            desc: 'write value into the top stack vector without popping it',
        },
        emit: { category: 'io', desc: 'commit poly; pops stack top into poly when present' },
        duplicate: { category: 'stack', desc: 'duplicate the top vector' },
        swap: { category: 'stack', desc: 'swap the top two vectors' },
        pop: { category: 'stack', desc: 'discard the top vector' },
        flush: { category: 'stack', desc: 'clear the vector stack' },
        blend: { category: 'vector', params: [{ ph: 't', def: '0.5', scalarExpr: 'real', title: 'Real scalar expression. Use real(p1) or imag(p2) when deriving from parameter registers.' }], desc: 'blend below*(1-t) + top*t for same-length vectors' },
        argsort: {
            category: 'vector',
            params: [
                { ph: 'tgt', def: 'poly', choices: _coeffProgramTargetChoices, selectorWide: true, title: 'Output target: write poly or push the sorted result.' },
                { ph: 'src1', def: 'poly', choices: _coeffProgramVectorSourceChoices, selectorWide: true, title: 'Vector to reorder.' },
                { ph: 'src2', def: 'poly', choices: _coeffProgramVectorSourceChoices, selectorWide: true, title: 'Sort key vector. Values are ordered by complex magnitude.' },
            ],
            desc: 'reorder src1 by ascending magnitude of src2',
        },
        roll: {
            category: 'vector',
            params: [
                { ph: 'tgt', def: 'poly', choices: _coeffProgramTargetChoices, selectorWide: true, title: 'Output target: write poly or push the rolled vector.' },
                { ph: 'src', def: 'poly', choices: _coeffProgramVectorSourceChoices, selectorWide: true, title: 'Input vector to roll left.' },
                { ph: 'n', def: '1', intLiteral: true, title: 'Integer roll amount. Negative values roll the other direction.' },
            ],
            desc: 'roll vector left by n positions',
        },
        rolr: {
            category: 'vector',
            params: [
                { ph: 'tgt', def: 'poly', choices: _coeffProgramTargetChoices, selectorWide: true, title: 'Output target: write poly or push the rolled vector.' },
                { ph: 'src', def: 'poly', choices: _coeffProgramVectorSourceChoices, selectorWide: true, title: 'Input vector to roll right.' },
                { ph: 'n', def: '1', intLiteral: true, title: 'Integer roll amount. Negative values roll the other direction.' },
            ],
            desc: 'roll vector right by n positions',
        },
        littlewood: {
            category: 'vector',
            params: [
                { ph: 'tgt', def: 'poly', choices: _coeffProgramTargetChoices, selectorWide: true, title: 'Output target: write poly or push the generated vector.' },
                { ph: 'value1', label: 'value1', def: '0', complex: true, scalarExpr: 'complex', complexWide: true, title: 'First complex value. Each coefficient randomly chooses value1 or value2 with 50/50 odds.' },
                { ph: 'value2', label: 'value2', def: '1', complex: true, scalarExpr: 'complex', complexWide: true, title: 'Second complex value. Each coefficient randomly chooses value1 or value2 with 50/50 odds.' },
                { ..._ctAndyParam, title: 'Blend generated values with current poly: output*(1-andy)+poly*andy.' },
            ],
            desc: 'generate a random Littlewood-style vector using current poly length',
        },
        macro: { category: 'macro', params: [{ ph: 'program id', def: '', programWide: true, title: 'Saved coeff-program id to expand at compile time.' }], desc: 'expand a saved coeff program at compile time' },
        legacy: {
            category: 'macro',
            hidden: true,
            params: [
                { ph: 'name', def: 'rev', choices: _coeffProgramLegacyNames, functionWide: true, title: 'Legacy coefficient transform function. Compiled to a stable numeric function index.' },
                { ph: 'src', def: 'poly', choices: _coeffProgramLegacySourceChoices, selectorWide: true, title: 'Input vector: cf read-only coefficients, current poly, pop stack, or peek stack.' },
                { ph: 'tgt', def: 'poly', choices: _coeffProgramTargetChoices, selectorWide: true, title: 'Output target: write poly or push the result onto the stack.' },
            ],
            desc: 'compatibility-only wrapper for old saved coeff-program chips',
        },
    };
    _coeffProgramVectorBinaryNames.forEach(name => {
        catalog[name] = {
            category: 'vector',
            params: [
                { ph: 'tgt', def: 'poly', choices: _coeffProgramTargetChoices, selectorWide: true, title: 'Output target: write poly or push the result.' },
                { ph: 'src1', def: 'poly', choices: _coeffProgramVectorSourceChoices, selectorWide: true, title: 'First input vector. pop consumes the current stack top.' },
                { ph: 'src2', def: 'poly', choices: _coeffProgramVectorSourceChoices, selectorWide: true, title: 'Second input vector. If src1 also pops, this sees the next stack value.' },
            ],
            desc: `${name}(src1, src2) elementwise vector operation`,
        };
    });
    _coeffProgramVectorUnaryNames.forEach(name => {
        catalog[name] = {
            category: 'vector',
            params: [
                { ph: 'tgt', def: 'poly', choices: _coeffProgramTargetChoices, selectorWide: true, title: 'Output target: write poly or push the result.' },
                { ph: 'src', def: 'poly', choices: _coeffProgramVectorSourceChoices, selectorWide: true, title: 'Input vector.' },
            ],
            desc: `${name}(src) vector operation`,
        };
    });
    Object.entries(_ctCatalog).forEach(([name, ctSpec]) => {
        if (name === 'exp') {
            catalog.exp_affine = {
                category: ctSpec.category || 'elementwise',
                nativeTransform: true,
                params: [
                    { ph: 'tgt', def: 'poly', choices: _coeffProgramTargetChoices, selectorWide: true, title: 'Output target: write poly or push the transform result.' },
                    { ph: 'src', def: 'poly', choices: _coeffProgramLegacySourceChoices, selectorWide: true, title: 'Input vector: cf read-only coefficients, current poly, pop stack, or peek stack.' },
                    ..._coeffProgramExpParamDefs.map(pDef => ({ ...pDef })).filter(pDef => !_isAndyParam(pDef)),
                ],
                label: 'exp_affine',
                desc: 'exp_affine(src, a, b): exp(src*a+b)',
            };
        }
        if (name === 'power') {
            // The registry's power transform is shadowed in this catalog by
            // the vector-binary power chip; expose it under the chip/source
            // name power_series instead.
            catalog.power_series = {
                category: ctSpec.category || 'elementwise',
                nativeTransform: true,
                params: [
                    { ph: 'tgt', def: 'poly', choices: _coeffProgramTargetChoices, selectorWide: true, title: 'Output target: write poly or push the transform result.' },
                    { ph: 'src', def: 'poly', choices: _coeffProgramLegacySourceChoices, selectorWide: true, title: 'Input vector: cf read-only coefficients, current poly, pop stack, or peek stack.' },
                    ..._coeffProgramLegacyInputDefs(name)
                        .filter(pDef => !_isAndyParam(pDef))
                        .map(pDef => ({ ...pDef })),
                ],
                label: 'power_series',
                desc: ctSpec.desc || 'power_series(src, k): (i+1) times a geometric series through z^k',
            };
        }
        if (catalog[name]) return;
        const argDefs = _coeffProgramLegacyInputDefs(name)
            .filter(pDef => !_isAndyParam(pDef))
            .map(pDef => ({ ...pDef }));
        catalog[name] = {
            category: ctSpec.category || 'elementwise',
            nativeTransform: true,
            params: [
                { ph: 'tgt', def: 'poly', choices: _coeffProgramTargetChoices, selectorWide: true, title: 'Output target: write poly or push the transform result.' },
                { ph: 'src', def: 'poly', choices: _coeffProgramLegacySourceChoices, selectorWide: true, title: 'Input vector: cf read-only coefficients, current poly, pop stack, or peek stack.' },
                ...argDefs,
            ],
            label: ctSpec.label || name,
            desc: ctSpec.desc || `${name}(src) native coefficient transform`,
        };
    });
    return catalog;
})();

function _ctParamValue(item, idx, pDefs) {
    const raw = item && Array.isArray(item.params) ? item.params[idx] : undefined;
    if (raw !== undefined && raw !== null && raw !== '') return String(raw);
    const def = pDefs && pDefs[idx] ? pDefs[idx].def : '';
    return def == null ? '' : String(def);
}

function _formatCoeffTransformComplexParts(reRaw, imRaw) {
    const re = _parseCtRealConstant(reRaw);
    const im = _parseCtRealConstant(imRaw);
    if (re == null || im == null) return `${String(reRaw || '0').trim()}+${String(imRaw || '0').trim()}j`;
    return _formatCfpvComplexValue(re, im);
}

function _normalizeCoeffTransformItem(item) {
    if (!item || !item.name) return item;
    const name = _canonicalCoeffTransformName(item.name);
    const params = Array.isArray(item.params) ? item.params.map(v => String(v)) : [];
    if (name === 'linear' && params.length >= 4) {
        const normalized = [
            _formatCoeffTransformComplexParts(params[0], params[1]),
            _formatCoeffTransformComplexParts(params[2], params[3]),
        ];
        if (params.length > 4) normalized.push(params[4]);
        return { name, params: normalized };
    }
    if (name === 'pow' && params.length >= 4) {
        const normalized = [
            _formatCoeffTransformComplexParts(params[0], params[1]),
            _formatCoeffTransformComplexParts(params[2], params[3]),
        ];
        if (params.length > 4) normalized.push(params[4]);
        return { name, params: normalized };
    }
    return { name, params };
}

function _ctAndyIndex(pDefs) {
    const idx = (pDefs || []).findIndex(_isAndyParam);
    // Catalog construction appends andy last, so the fallback matches it.
    return idx >= 0 ? idx : Math.max(0, (pDefs || []).length - 1);
}

function _ctAndyHtml(which, chipIdx, item, pDefs, options = {}) {
    const idx = _ctAndyIndex(pDefs);
    return `<span class="chip-op">andy=</span>${_chipInputHtml(which, chipIdx, idx, _ctParamValue(item, idx, pDefs), pDefs[idx] || _ctAndyParam, options)}`;
}

function _formatCtConstant(value) {
    if (!Number.isFinite(value)) return null;
    return Number(value.toPrecision(15)).toString();
}

function _parseCtRealConstant(value) {
    const s = _str(value).replace(/\s+/g, '');
    if (!s) return null;
    let idx = 0;
    let total = 0;
    let saw = false;
    const numberRe = /^[+-]?(?:(?:\d+(?:\.\d*)?)|(?:\.\d+))(?:[eE][+-]?\d+)?/;
    while (idx < s.length) {
        const rest = s.slice(idx);
        const match = rest.match(numberRe);
        if (!match) return null;
        total += Number(match[0]);
        if (!Number.isFinite(total)) return null;
        saw = true;
        idx += match[0].length;
        if (idx >= s.length) break;
        const ch = s[idx];
        if (ch !== '+' && ch !== '-') return null;
    }
    return saw ? total : null;
}

function _hasCtExpressionOperator(value) {
    const s = _str(value).replace(/\s+/g, '');
    for (let i = 1; i < s.length; i++) {
        if ((s[i] === '+' || s[i] === '-') && s[i - 1] !== 'e' && s[i - 1] !== 'E') return true;
    }
    return false;
}

function _normalizeCtRealInput(value) {
    const raw = _str(value).trim();
    if (!raw) return raw;
    const parsed = _parseCtRealConstant(raw);
    if (parsed == null) return null;
    return _hasCtExpressionOperator(raw) ? _formatCtConstant(parsed) : raw;
}

function _parseCtComplexConstant(value) {
    const s = _str(value).replace(/\s+/g, '').replace(/i/gi, 'j');
    if (!s) return null;
    let re = 0;
    let im = 0;
    let saw = false;
    let sawImag = false;
    let start = 0;
    const terms = [];
    for (let i = 1; i < s.length; i++) {
        if ((s[i] === '+' || s[i] === '-') && s[i - 1] !== 'e' && s[i - 1] !== 'E') {
            terms.push(s.slice(start, i));
            start = i;
        }
    }
    terms.push(s.slice(start));
    for (const term of terms) {
        if (!term) return null;
        if (term.includes('j')) {
            if (!term.endsWith('j') || term.indexOf('j') !== term.length - 1) return null;
            let coeff = term.slice(0, -1);
            if (!coeff || coeff === '+') coeff = '1';
            else if (coeff === '-') coeff = '-1';
            const parsed = _parseCtRealConstant(coeff);
            if (parsed == null) return null;
            im += parsed;
            sawImag = true;
            saw = true;
        } else {
            const parsed = _parseCtRealConstant(term);
            if (parsed == null) return null;
            re += parsed;
            saw = true;
        }
    }
    if (!saw || !Number.isFinite(re) || !Number.isFinite(im)) return null;
    return { re, im, sawImag };
}

function _splitCtComplexInput(value) {
    const parsed = _parseCtComplexConstant(value);
    if (!parsed || !parsed.sawImag) return null;
    return { re: _formatCtConstant(parsed.re), im: _formatCtConstant(parsed.im) };
}

function _ctAndyIsDefault(value) {
    const v = Number(value);
    return Number.isFinite(v) && Math.abs(v) < 1e-15;
}

function _serializeCoeffTransforms() {
    return _ctChain.map(item => _normalizeCoeffTransformItem(item)).map(item => {
        if (!item || !item.name) return null;
        const spec = _ctCatalog[item.name] || {};
        const pDefs = spec.params || [];
        const values = pDefs.map((pDef, idx) => _ctParamValue(item, idx, pDefs));
        if (!values.length) return item.name;
        let last = values.length - 1;
        if (_ctAndyIsDefault(values[last])) last--;
        if (last < 0) return item.name;
        return [item.name, ...values.slice(0, last + 1)];
    }).filter(Boolean);
}

function _ctCategoryGroups() {
    const grouped = {};
    Object.keys(_ctCategoryMeta).forEach(key => { grouped[key] = []; });
    Object.keys(_ctCatalog).forEach(name => {
        const spec = _ctCatalog[name] || {};
        const key = grouped[spec.category] ? spec.category : 'elementwise';
        grouped[key].push(name);
    });
    return Object.keys(_ctCategoryMeta).map(key => ({
        key,
        ..._ctCategoryMeta[key],
        items: grouped[key] || [],
    }));
}

function _renderCoeffTransformAddPopup() {
    const popup = document.getElementById('ct-add-popup');
    if (!popup) return;
    const head = `<div class="score-chip-picker-head"><span class="score-chip-picker-title">Add coeff transform</span><span class="score-chip-picker-state">andy blends f(z) with original z</span></div>`;
    const body = _ctCategoryGroups().map(group => {
        const items = group.items.map(name => {
            const spec = _ctCatalog[name] || {};
            const label = spec.label || name;
            const paramCount = Math.max(0, (spec.params || []).length - 1);
            const params = paramCount ? ` · ${_pluralize(paramCount, 'param')} + andy` : ' · andy';
            return `<button type="button" class="score-chip-option score-chip-option-${_escapeHtml(group.key)}" onclick="selectCoeffTransformChip('${_escapeHtml(name)}',event)" title="${_escapeHtml(spec.desc || label)}"><span class="score-chip-option-name">${_escapeHtml(label)}</span><span class="score-chip-option-meta">${_escapeHtml((spec.desc || '') + params)}</span></button>`;
        }).join('');
        return `<div class="score-chip-category"><div class="score-chip-category-title">${_escapeHtml(group.title)}</div><div class="score-chip-category-help">${_escapeHtml(group.help)}</div><div class="score-chip-options">${items}</div></div>`;
    }).join('');
    popup.innerHTML = head + body;
}

function _setCoeffTransformPickerOpen(open) {
    const popup = document.getElementById('ct-add-popup');
    const btn = document.getElementById('ct-add-btn');
    if (!popup) return;
    if (open) _renderCoeffTransformAddPopup();
    popup._open = !!open;
    popup.style.display = open ? 'block' : 'none';
    if (popup.classList && popup.classList.toggle) popup.classList.toggle('active', !!open);
    if (popup.setAttribute) popup.setAttribute('aria-hidden', open ? 'false' : 'true');
    if (btn && btn.setAttribute) btn.setAttribute('aria-expanded', open ? 'true' : 'false');
}

function toggleCoeffTransformPicker(eventObj) {
    if (eventObj && eventObj.stopPropagation) eventObj.stopPropagation();
    const popup = document.getElementById('ct-add-popup');
    _setCoeffTransformPickerOpen(!(popup && popup._open));
}

function selectCoeffTransformChip(name, eventObj) {
    if (eventObj && eventObj.stopPropagation) eventObj.stopPropagation();
    addChip('ct', name);
    _setCoeffTransformPickerOpen(false);
}

function _syncCoeffTransformAddOptions() {
    const sel = document.getElementById('ct-add');
    if (sel) {
        const options = ['<option value="">+ add...</option>'].concat(
            Object.keys(_ctCatalog).map(name => {
                const spec = _ctCatalog[name] || {};
                return `<option value="${_escapeHtml(name)}">${_escapeHtml(spec.label || name)}</option>`;
            })
        );
        sel.innerHTML = options.join('');
    }
    _renderCoeffTransformAddPopup();
}

let _ptChain = [];  // parameter transform chain (array of {name, params: [str,...]})
let _ppChain = [];  // parameter program chain (array of {name, params: [str,...]})
let _paramPipelineMode = 'chain';
let _ctChain = [];  // coefficient transform chain (array of {name, params: [str,...]})
let _coeffProgramChain = [];  // coefficient program chain (array of {name, params: [str,...]})
let _cfpv = [];     // coefficient function parameter vector (array of doubles)
let _rtChain = [];  // root transform chain (array of {name, params: [str,...]})
let _paletteRtChain = [];  // palette-tab root transform chain
let _renderScoreChain = [];
let _paletteScoreChain = [];

function _getCatalogEntry(name) {
    const cat = window._coeffFuncCatalog || [];
    return cat.find(e => e.name === name) || null;
}

function _isConstCoeffFunction(entryOrName) {
    const name = typeof entryOrName === 'string' ? entryOrName : (entryOrName && entryOrName.name);
    return String(name || '') === 'const';
}

function _constCoeffDefaults(entry) {
    const params = (entry && entry.params) || [];
    const length = Math.max(1, Math.round(Number(params[0]?.default ?? 35) || 35));
    const re = Number(params[1]?.default ?? 1);
    const im = Number(params[2]?.default ?? 0);
    return {
        degree: Math.max(0, length - 1),
        length,
        re: Number.isFinite(re) ? re : 1,
        im: Number.isFinite(im) ? im : 0,
    };
}

function _coeffFuncUiParamCount(entry) {
    if (_isConstCoeffFunction(entry)) return 2;
    return ((entry && entry.params) || []).length;
}

function _formatCfpvComplexValue(re, im) {
    const r = Number(re);
    const i = Number(im);
    const reText = _formatCtConstant(Number.isFinite(r) ? r : 0);
    const imVal = Number.isFinite(i) ? i : 0;
    if (Math.abs(imVal) < 1e-15) return reText;
    const absImText = _formatCtConstant(Math.abs(imVal));
    const imText = absImText === '1' ? 'j' : `${absImText}j`;
    if (Math.abs(Number(reText)) < 1e-15) return imVal < 0 ? `-${imText}` : imText;
    return `${reText}${imVal < 0 ? '-' : '+'}${imText}`;
}

function _parseCfpvComplexValue(raw) {
    const parsed = _parseCtComplexConstant(raw);
    if (!parsed) return null;
    return {
        re: Number(_formatCtConstant(parsed.re)),
        im: Number(_formatCtConstant(parsed.im)),
    };
}

function _formatCfpvForDisplay(funcName, cfpv) {
    const vals = Array.isArray(cfpv) ? cfpv : [];
    if (!vals.length) return '';
    if (_isConstCoeffFunction(funcName)) {
        const length = Math.max(1, Math.round(Number(vals[0]) || 1));
        const degree = Math.max(0, length - 1);
        const re = vals.length > 1 ? vals[1] : 1;
        const im = vals.length > 2 ? vals[2] : 0;
        return `degree=${degree}, value=${_formatCfpvComplexValue(re, im)}`;
    }
    const entry = _getCatalogEntry(funcName);
    const params = (entry && entry.params) || [];
    return vals.map((v, i) => {
        const p = params[i] || {};
        const label = p.label || p.name || `p${i + 1}`;
        return `${label}=${v}`;
    }).join(', ');
}

function _setConstCoeffInputsFromRaw(cfpv) {
    const defaults = _constCoeffDefaults(_getCatalogEntry('const'));
    const raw = Array.isArray(cfpv) ? cfpv : [];
    const length = Math.max(1, Math.round(Number(raw[0] ?? defaults.length) || defaults.length));
    const re = Number(raw[1] ?? defaults.re);
    const im = Number(raw[2] ?? defaults.im);
    const degreeInp = document.getElementById('cfpv-p0');
    const valueInp = document.getElementById('cfpv-p1');
    if (degreeInp) degreeInp.value = String(Math.max(0, length - 1));
    if (valueInp) valueInp.value = _formatCfpvComplexValue(
        Number.isFinite(re) ? re : defaults.re,
        Number.isFinite(im) ? im : defaults.im
    );
}

function _coeffFuncLabel(entry) {
    if (!entry) return '';
    let label = entry.name;
    if (entry.stubbed) label += ' [stub]';
    else if (entry.agreement_pct !== undefined) label += ` [${entry.agreement_pct}%]`;
    label += entry.probe_failed ? ' (degree ?)' : ` (degree ${entry.degree})`;
    label += ` (${entry.kind})`;
    if (entry.params && entry.params.length) label += ' *';
    return label;
}

function _coeffFuncMeta(entry) {
    if (!entry) return '';
    const parts = [];
    if (!entry.probe_failed && entry.degree != null) parts.push(`degree ${entry.degree}`);
    if (entry.kind) parts.push(entry.kind);
    if (entry.agreement_pct !== undefined) parts.push(`${entry.agreement_pct}%`);
    if (entry.parity_verified) {
        const cases = entry.parity_cases != null ? ` ${entry.parity_cases} case${entry.parity_cases === 1 ? '' : 's'}` : '';
        parts.push(`verified${cases}`);
    }
    const paramCount = _coeffFuncUiParamCount(entry);
    if (paramCount) parts.push(paramCount + ' param' + (paramCount === 1 ? '' : 's'));
    return parts.join(' • ');
}

function _syncRenderFunctionPicker() {
    const btn = document.getElementById('render-function-picker');
    const meta = document.getElementById('render-function-picker-meta');
    const sel = document.getElementById('render-function');
    const entry = _getCatalogEntry(sel && sel.value ? sel.value : '');
    if (btn) {
        btn.textContent = entry ? entry.name : 'Choose coefficient function...';
        btn.title = entry ? _coeffFuncLabel(entry) : 'Choose coefficient function';
        btn.disabled = !(window._coeffFuncCatalog || []).length;
    }
    if (meta) meta.textContent = entry ? _coeffFuncMeta(entry) : '';
}

function _setRenderFunction(name) {
    const sel = document.getElementById('render-function');
    if (!sel) return false;
    const entry = _getCatalogEntry(name);
    if (!entry) {
        _syncRenderFunctionPicker();
        updateCfpvRow();
        return false;
    }
    sel.value = name;
    _syncRenderFunctionPicker();
    updateCfpvRow();
    _markComputePreviewStale();
    return true;
}

function _functionCatalogSearchFields(entry) {
    return [
        entry.name || '',
        entry.kind || '',
        entry.source || '',
        entry.degree != null ? String(entry.degree) : '',
        entry.agreement_pct != null ? String(entry.agreement_pct) : '',
        entry.parity_verified ? 'verified' : '',
        entry.parity_cases != null ? String(entry.parity_cases) : '',
        _coeffFuncUiParamCount(entry) ? 'params' : '',
    ];
}

function _functionFilterMatcher(rawFilter) {
    const raw = String(rawFilter || '').trim();
    if (!raw) return { mode: 'all', error: '', test: () => true };
    if (/[\\^$.*+?()[\]{}|]/.test(raw)) {
        try {
            const regex = new RegExp(raw, 'i');
            return {
                mode: 'regex',
                error: '',
                test: entry => _functionCatalogSearchFields(entry).some(field => regex.test(String(field))),
            };
        } catch (err) {
            return {
                mode: 'invalid_regex',
                error: err && err.message ? err.message : 'invalid regex',
                test: () => false,
            };
        }
    }
    const needle = raw.toLowerCase();
    return {
        mode: 'text',
        error: '',
        test: entry => _functionCatalogSearchFields(entry).join(' ').toLowerCase().includes(needle),
    };
}

function _visibleFunctionCatalog() {
    const cat = window._coeffFuncCatalog || [];
    const matcher = _functionFilterMatcher(_functionPopupState.filter || '');
    if (matcher.mode === 'all') return cat;
    return cat.filter(entry => matcher.test(entry));
}

function populateDropdown() {
    const sel = document.getElementById('render-function');
    const pickerBtn = document.getElementById('render-function-picker');
    const current = sel && sel.value ? sel.value : '';
    sel.innerHTML = '';
    const cat = window._coeffFuncCatalog || [];
    if (!cat.length) {
        const opt = document.createElement('option');
        opt.value = '';
        opt.textContent = 'ERROR: catalog failed to load';
        opt.style.color = 'red';
        sel.appendChild(opt);
        if (pickerBtn) {
            pickerBtn.textContent = 'ERROR: catalog failed to load';
            pickerBtn.disabled = true;
        }
        const meta = document.getElementById('render-function-picker-meta');
        if (meta) meta.textContent = '';
        console.error('coeff_func_catalog_js.js did not load — dropdown is empty');
        return;
    }
    for (const e of cat) {
        const opt = document.createElement('option');
        opt.value = e.name;
        opt.textContent = _coeffFuncLabel(e);
        sel.appendChild(opt);
    }
    if (current && _getCatalogEntry(current)) sel.value = current;
    else if (!sel.value && cat[0]) sel.value = cat[0].name;
    _syncRenderFunctionPicker();
    updateCfpvRow();
}

function parseCfpv() {
    const entry = _getCatalogEntry(document.getElementById('render-function').value);
    const params = (entry && entry.params) || [];
    if (_isConstCoeffFunction(entry)) {
        const defaults = _constCoeffDefaults(entry);
        const degreeInp = document.getElementById('cfpv-p0');
        const valueInp = document.getElementById('cfpv-p1');
        const degreeRaw = String(degreeInp ? degreeInp.value : '').trim();
        const valueRaw = String(valueInp ? valueInp.value : '').trim();
        const degree = degreeRaw ? Number(degreeRaw) : defaults.degree;
        const value = valueRaw ? _parseCfpvComplexValue(valueRaw) : { re: defaults.re, im: defaults.im };
        const degreeValid = Number.isFinite(degree) && Number.isInteger(degree) && degree >= 0;
        const valueValid = !!value && Number.isFinite(value.re) && Number.isFinite(value.im);
        if (degreeInp) degreeInp.style.borderColor = degreeValid ? '' : 'red';
        if (valueInp) valueInp.style.borderColor = valueValid ? '' : 'red';
        _cfpv = (degreeValid && valueValid) ? [degree + 1, value.re, value.im] : [];
        _markComputePreviewStale();
        return;
    }
    const vals = [];
    let valid = true;
    for (let i = 0; i < params.length; i++) {
        const inp = document.getElementById('cfpv-p' + i);
        if (!inp) continue;
        const raw = inp.value.trim();
        if (!raw) {
            vals.push(params[i].default);
            inp.style.borderColor = '';
            continue;
        }
        const v = Number(raw);
        if (!Number.isFinite(v)) {
            inp.style.borderColor = 'red';
            valid = false;
        } else {
            inp.style.borderColor = '';
            vals.push(v);
        }
    }
    _cfpv = valid ? vals : [];
    _markComputePreviewStale();
}

function updateCfpvRow() {
    const funcName = document.getElementById('render-function').value;
    const entry = _getCatalogEntry(funcName);
    _syncRenderFunctionPicker();
    const row = document.getElementById('cfpv-row');
    const container = document.getElementById('cfpv-inputs');
    const params = (entry && entry.params) || [];
    if (params.length === 0) {
        row.style.display = 'none';
        _cfpv = [];
        container.innerHTML = '';
        return;
    }
    row.style.display = '';
    container.innerHTML = '';
    if (_isConstCoeffFunction(entry)) {
        const defaults = _constCoeffDefaults(entry);
        const degreeLbl = document.createElement('span');
        degreeLbl.style.fontSize = '11px';
        degreeLbl.style.color = '#888';
        degreeLbl.textContent = 'degree:';
        const degreeInp = document.createElement('input');
        degreeInp.id = 'cfpv-p0';
        degreeInp.type = 'number';
        degreeInp.min = '0';
        degreeInp.step = '1';
        degreeInp.value = String(defaults.degree);
        degreeInp.style.width = '70px';
        degreeInp.style.fontSize = '12px';
        degreeInp.title = 'Polynomial degree; native coefficient length is degree + 1.';
        degreeInp.onchange = parseCfpv;
        const valueLbl = document.createElement('span');
        valueLbl.style.fontSize = '11px';
        valueLbl.style.color = '#888';
        valueLbl.textContent = 'value:';
        const valueInp = document.createElement('input');
        valueInp.id = 'cfpv-p1';
        valueInp.type = 'text';
        valueInp.value = _formatCfpvComplexValue(defaults.re, defaults.im);
        valueInp.placeholder = '1-2j';
        valueInp.style.width = '120px';
        valueInp.style.fontSize = '12px';
        valueInp.title = 'Complex constant value, e.g. 1-2j or 10j-3.';
        valueInp.onchange = parseCfpv;
        container.appendChild(degreeLbl);
        container.appendChild(degreeInp);
        container.appendChild(valueLbl);
        container.appendChild(valueInp);
        _cfpv = [defaults.length, defaults.re, defaults.im];
        _markComputePreviewStale();
        return;
    }
    const defaults = [];
    for (let i = 0; i < params.length; i++) {
        const p = params[i];
        const lbl = document.createElement('span');
        lbl.style.fontSize = '11px';
        lbl.style.color = '#888';
        lbl.textContent = (p.label || p.name || `p${i + 1}`) + ':';
        const inp = document.createElement('input');
        inp.id = 'cfpv-p' + i;
        inp.type = 'number';
        inp.value = p.default;
        inp.style.width = '60px';
        inp.style.fontSize = '12px';
        if (p.min !== undefined) inp.min = p.min;
        if (p.max !== undefined) inp.max = p.max;
        inp.onchange = parseCfpv;
        container.appendChild(lbl);
        container.appendChild(inp);
        defaults.push(p.default);
    }
    _cfpv = defaults;
    _markComputePreviewStale();
}

function addChip(which, name, insertMode = 'append') {
    if (!name) return;
    if (which === 'ct') name = _canonicalCoeffTransformName(name);
    const chain = _chainForWhich(which);
    let newItem = null;
    if (which === 'pt' || which === 'pp' || which === 'cp' || which === 'ct' || which === 'rt' || which === 'palette-rt' || which === 'ss' || which === 'palette-ss') {
        if ((which === 'ss' || which === 'palette-ss') && !_solveScoreAllowedAdditions(which).includes(name)) {
            return;
        }
        const catalog = _catalogForChain(which);
        const spec = catalog[name] || {};
        const p = spec.params || [];
        newItem = { name, params: p.map(d => d.def || '') };
    } else {
        newItem = name;
    }
    let insertAt = chain.length;
    if ((which === 'ss' || which === 'palette-ss' || which === 'pp' || which === 'cp') && (insertMode === 'before' || insertMode === 'after')) {
        const selected = which === 'pp'
            ? Number(_paramProgramSelectedIndex)
            : (which === 'cp' ? Number(_coeffProgramSelectedIndex) : Number(_solveScoreSelectedIndex[which]));
        if (Number.isInteger(selected) && selected >= 0 && selected < chain.length) {
            insertAt = insertMode === 'before' ? selected : selected + 1;
        }
    }
    chain.splice(insertAt, 0, newItem);
    if (which === 'ss' || which === 'palette-ss') _solveScoreSelectedIndex[which] = insertAt;
    if (which === 'pp') _paramProgramSelectedIndex = insertAt;
    if (which === 'cp') _coeffProgramSelectedIndex = insertAt;
    _renderChips(which);
    if (_paramPipelineEditAffectsCompute(which)) _markComputePreviewStale();
}

function removeChip(which, idx) {
    const chain = _chainForWhich(which);
    chain.splice(idx, 1);
    if (which === 'ss' || which === 'palette-ss') {
        const selected = Number(_solveScoreSelectedIndex[which]);
        _solveScoreSelectedIndex[which] = selected === idx ? -1 : (selected > idx ? selected - 1 : selected);
    }
    if (which === 'pp') {
        const selected = Number(_paramProgramSelectedIndex);
        _paramProgramSelectedIndex = selected === idx ? -1 : (selected > idx ? selected - 1 : selected);
    }
    if (which === 'cp') {
        const selected = Number(_coeffProgramSelectedIndex);
        _coeffProgramSelectedIndex = selected === idx ? -1 : (selected > idx ? selected - 1 : selected);
    }
    _renderChips(which);
    if (_paramPipelineEditAffectsCompute(which)) _markComputePreviewStale();
}

function moveChip(which, idx, delta) {
    const chain = _chainForWhich(which);
    const from = Number(idx);
    const to = from + Number(delta);
    if (!Number.isInteger(from) || !Number.isInteger(to)) return;
    if (from < 0 || from >= chain.length || to < 0 || to >= chain.length) return;
    const [item] = chain.splice(from, 1);
    chain.splice(to, 0, item);
    if (which === 'ss' || which === 'palette-ss') _solveScoreSelectedIndex[which] = to;
    if (which === 'pp') _paramProgramSelectedIndex = to;
    if (which === 'cp') _coeffProgramSelectedIndex = to;
    _renderChips(which);
    if (_paramPipelineEditAffectsCompute(which)) _markComputePreviewStale();
}

function _normalizeTarget(value) {
    const v = value.trim().toLowerCase();
    if (v === 't1' || v === '0') return 't1';
    if (v === 't2' || v === '1') return 't2';
    if (v === 'both' || v === 't1,t2' || v === 't2,t1' || v === '2') return 'both';
    return null; // invalid
}

function _targetToWire(sym) {
    if (sym === 't1') return '0';
    if (sym === 't2') return '1';
    if (sym === 'both') return '2';
    return '0';
}

;(window.__ppParts = window.__ppParts || []).push('07-transform-catalogs');
