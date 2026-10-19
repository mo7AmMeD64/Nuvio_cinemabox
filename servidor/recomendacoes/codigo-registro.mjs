#!/usr/bin/env node
// Codigo de registro <-> id da tabela `registro` (ver codigoRegistro em
// src/index.js e src/regcodigo.c no cliente).
//
//   node codigo-registro.mjs K7QM2X     -> id da linha
//   node codigo-registro.mjs 9866       -> codigo que a TV mostrou
//
// Com o id: wrangler d1 execute <banco> --remote \
//   --command "SELECT id, pessoa, versao, plataforma, quando FROM registro WHERE id = <id>"
import { codigoRegistro } from "./src/codigo.js";
const ALFA = "0123456789ABCDEFGHJKMNPQRSTVWXYZ";
const A_INV = 0x39331415n, B = 0x15a4e3c7n, M = 1n << 30n;
const arg = (process.argv[2] || "").trim().toUpperCase()
  .replace(/O/g, "0").replace(/[IL]/g, "1");   // leitura de Crockford
if (/^\d+$/.test(arg)) { console.log(codigoRegistro(arg)); process.exit(0); }
if (!/^[0-9A-HJKMNP-TV-Z]{6}$/.test(arg)) { console.error("uso: codigo-registro.mjs <codigo de 6 | id>"); process.exit(1); }
let v = 0n;
for (const c of arg) v = v * 32n + BigInt(ALFA.indexOf(c));
console.log(String((((v - B) % M + M) % M * A_INV) % M));
