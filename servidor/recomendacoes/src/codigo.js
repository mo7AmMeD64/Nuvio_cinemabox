// CODIGO DO REGISTRO (Ajustes > Enviar registro, 03/10): seis caracteres que a
// pessoa dita no grupo ou cola na issue. E o id da linha numa bijecao
// afim mod 2^30 em base32 de Crockford (sem I, L, O, U): nao e sequencial na
// tela, nao precisa de coluna nova, e volta ao id com
// codigo-registro.mjs.
// A TV calcula o MESMO codigo a partir do registro_id (src/regcodigo.c) quando
// o servidor ainda nao manda este campo; mudar as constantes aqui exige mudar
// la tambem, senao suporte e TV leem codigos diferentes.
const COD_ALFA = "0123456789ABCDEFGHJKMNPQRSTVWXYZ";
const COD_A = 0x2c5e1b3dn, COD_B = 0x15a4e3c7n, COD_M = 1n << 30n;
export function codigoRegistro(id) {
  const n = BigInt(id);
  if (n < 1n || n >= COD_M) return null;
  let v = (n * COD_A + COD_B) % COD_M, s = "";
  for (let i = 0; i < 6; i++) { s = COD_ALFA[Number(v & 31n)] + s; v >>= 5n; }
  return s;
}
