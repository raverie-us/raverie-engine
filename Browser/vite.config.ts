import { defineConfig, Plugin } from "vite";
import * as fs from "fs";
import * as mimetypes from "mime-types";

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
    outDir: "../docs/"
  },
  plugins: [dataUrlLoader, rawBufferLoader]
});
