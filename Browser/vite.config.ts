import { defineConfig } from "vite";
import nodePolyfills from "vite-plugin-node-stdlib-browser";

export default defineConfig({
  server: {
    port: 8080,
  },
  plugins: [nodePolyfills()]
});
