// Playwright config — no local npm install needed, runs via npx
// Usage: npx playwright test
module.exports = {
  testDir: './tests/e2e',
  timeout: 30000,
  use: {
    headless: true,
  },
  webServer: {
    command: 'python3 -m http.server 8765 --directory .',
    port: 8765,
    reuseExistingServer: true,
  },
};
