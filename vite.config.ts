import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';
import { libraryContentPlugin } from './vite-plugins/libraryContentPlugin.ts';

export default defineConfig(({ command }) => ({
  base: command === 'build' ? '/library/' : '/',
  plugins: [react(), libraryContentPlugin()],
}));
