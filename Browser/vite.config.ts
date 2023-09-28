import { defineConfig, Plugin } from "vite";
import dts from "vite-plugin-dts";
import * as fs from "fs";
import * as mimetypes from "mime-types";
import { resolve } from "path";

const dataUrlLoader: Plugin = {
  name: "data-url-loader",
  transform: (code: string, id: string) => {
      const [path, query] = id.split("?");
      if (query != "data-url")
          return null;

      const data = fs.readFileSync(path);
      const base64 = data.toString("base64");
      const mimeType = mimetypes.lookup(path);
      const dataUrl = `data:${mimeType};base64,${base64}`;
      return `export default "${dataUrl}";`;
  }
};

const rawBufferLoader: Plugin = {
  name: "raw-buffer-loader",
  transform: (code: string, id: string) => {
      const [path, query] = id.split("?");
      if (query != "raw-buffer")
          return null;

      const data = fs.readFileSync(path);
      return `export default new Uint8Array([${data.join(",")}]);`;
  }
};

export default defineConfig({
  server: {
    port: 8080,
  },
  build: {
    outDir: process.env.PAGES_BUILD ? "../docs/" : "../Build/Browser",
    emptyOutDir: true,
    assetsDir: "",
    rollupOptions: {
      preserveEntrySignatures: "strict",
      output: {
        entryFileNames: `[name].js`,
        chunkFileNames: `[name].js`,
        assetFileNames: `[name].[ext]`
      },
      input: {
        "raverie-engine": resolve(__dirname, "main.ts"),
        "raverie-engine-worker": resolve(__dirname, "worker.ts"),
        "page": resolve(__dirname, "index.html"),
      }
    }
  },
  base: "",
  plugins: [dts({ rollupTypes: true }), dataUrlLoader, rawBufferLoader]
});
