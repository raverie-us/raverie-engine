module.exports = {
  extends: [
    "eslint:all"
  ],
  parserOptions: {
    ecmaVersion: 2018,
    sourceType: "module"
  },
  env: {
    node: true,
    es6: true
  },
  rules: {
    "indent": ["error", 2],
    "no-extra-parens": "off",

    "max-statements": ["error", 100],
    "max-lines-per-function": ["error", 200],
    "max-len": ["error", 160],
    "padded-blocks": ["error", "never"],
    "object-property-newline": "off",
    "object-curly-newline": ["error", { "multiline": true, "consistent": true }],
    "multiline-ternary": "off",
    
    "no-console": "off",
    "no-process-env": "off",

    "quote-props": ["error", "consistent-as-needed"],
    "one-var": "off",
    "no-ternary": "off",
    "no-confusing-arrow": "off",
    "no-await-in-loop": "off",
    "no-magic-numbers": "off",
    "no-new": "off",
    "require-await": "off",
    "class-methods-use-this": "off",
    "global-require": "off",
    "callback-return": "off",
    "no-plusplus": "off",
    "max-params": ["error", 6],

    "default-case": "off",
    "max-lines": ["error", 1000],
    "no-sync": "off",
    "no-continue": "off",
    "no-loop-func": "off",
    "lines-around-comment": "off"
  }
};
