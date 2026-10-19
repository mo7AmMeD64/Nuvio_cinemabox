#ifndef NV_COLFILEIRAS_H
#define NV_COLFILEIRAS_H
// Main-thread publication of collection identities into the local Home editor.
// Does not push anything to the account or overwrite personal row choices.
void colfileiras_sincronizar(void);
int colfileiras_receber(const char *json);
void colfileiras_contexto(void);
#endif
