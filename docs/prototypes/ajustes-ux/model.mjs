/**
 * Deliberately small, in-memory model for the settings UX prototype.
 * Values and defaults describe the demo, not a device capability assessment.
 * Nothing here writes app preferences, contacts a service or starts a timer.
 */

function freeze(value) {
  if (value && typeof value === 'object') {
    Object.values(value).forEach(freeze);
    Object.freeze(value);
  }
  return value;
}

const choice = (value, label, description) => ({ value, label, description });
const toggle = (on, off) => [
  choice('on', 'Ligado', on),
  choice('off', 'Desligado', off),
];

export const SECTIONS = freeze([
  { id: 'trailers', label: 'Trailers', description: 'Escolha onde os trailers aparecem e quando têm som.' },
  { id: 'languages', label: 'Idiomas e legendas', description: 'Interface, áudio, legendas e textos dos títulos têm escolhas independentes.' },
  { id: 'performance', label: 'Desempenho desta TV', description: 'Ajuste a interface e as imagens deste aparelho.' },
]);

export const OPTIONS = freeze([
  {
    id: 'heroTrailer', section: 'trailers', group: 'Onde toca',
    label: 'No destaque do topo',
    description: 'Quando o foco para no destaque, um trailer disponível pode substituir a imagem. Desligado mantém a arte.',
    scope: 'profile', defaultValue: 'on',
    values: toggle('Mostra o trailer no lugar da arte do destaque, quando disponível.', 'Mantém a imagem do destaque.'),
    advanced: false, synonyms: ['trailer', 'hero', 'automático', 'autoplay', 'fundo'], preview: 'trailer',
  },
  {
    id: 'focusTrailer', section: 'trailers', group: 'Onde toca',
    label: 'No cartaz em foco',
    description: 'Ao parar num cartaz, mostra um trailer sem som. No app, exige cartaz expandido ou horizontal; o modo seguro pode suspender o efeito.',
    scope: 'profile', defaultValue: 'off',
    values: toggle('Mostra o trailer sem som após a espera do cartaz.', 'Mantém a arte ao parar num cartaz.'),
    advanced: false, synonyms: ['trailer', 'pôster', 'poster', 'cartaz', 'foco', 'autoplay'], preview: 'trailer',
  },
  {
    id: 'detailTrailer', section: 'trailers', group: 'Onde toca',
    label: 'Na página do título',
    description: 'Um trailer disponível pode começar ao abrir um título. Desligado mantém a imagem de fundo.',
    scope: 'profile', defaultValue: 'on',
    values: toggle('Inicia o trailer disponível na página do título.', 'Mantém a arte de fundo da página do título.'),
    advanced: false, synonyms: ['trailer', 'detalhes', 'filme', 'série', 'autoplay'], preview: 'trailer',
  },
  {
    id: 'heroSound', section: 'trailers', group: 'Som e imagem',
    label: 'Som no destaque',
    description: 'Escolhe se o trailer do destaque toca com som. No app, o áudio também depende da TV e da fonte do trailer.',
    scope: 'device', defaultValue: 'off',
    values: toggle('Permite áudio no trailer do destaque quando a TV e a fonte suportam.', 'Mantém o trailer do destaque sem som.'),
    advanced: false, synonyms: ['trailer', 'mudo', 'silêncio', 'áudio', 'volume'], preview: 'trailer', requires: 'heroTrailer',
  },
  {
    id: 'detailSound', section: 'trailers', group: 'Som e imagem',
    label: 'Som na página do título',
    description: 'Escolhe o som do trailer automático na página do título. No app, a TV e a fonte precisam oferecer áudio.',
    scope: 'device', defaultValue: 'off',
    values: toggle('Permite áudio no trailer automático do título quando suportado.', 'Mantém o trailer automático do título sem som.'),
    advanced: false, synonyms: ['trailer', 'detalhes', 'mudo', 'áudio', 'volume'], preview: 'trailer', requires: 'detailTrailer',
  },
  {
    id: 'trailerSource', section: 'trailers', group: 'Som e imagem',
    label: 'Fonte do trailer',
    description: 'Automático procura entre as fontes compatíveis. Uma fonte escolhida limita a busca; disponibilidade e áudio variam por TV.',
    scope: 'device', defaultValue: 'auto',
    values: [
      choice('auto', 'Automático', 'Usa a ordem de fontes compatível com a TV.'),
      choice('apple', 'Apple TV', 'Procura somente trailers da Apple TV.'),
      choice('imdb', 'IMDb', 'Procura somente trailers do IMDb.'),
    ],
    advanced: false, synonyms: ['trailer', 'provedor', 'origem', 'apple', 'imdb'], preview: 'trailer',
  },
  {
    id: 'trailerDelay', section: 'trailers', group: 'Som e imagem',
    label: 'Espera no destaque',
    description: 'Tempo parado no destaque antes de trocar a arte pelo trailer. Não é a espera do cartaz em foco.',
    scope: 'device', defaultValue: '5',
    values: [
      choice('3', '3 segundos', 'Começa mais cedo quando o foco permanece no destaque.'),
      choice('5', '5 segundos', 'Dá tempo para olhar a arte antes do trailer.'),
      choice('8', '8 segundos', 'Espera mais antes de mostrar o trailer.'),
    ],
    advanced: true, synonyms: ['trailer', 'demora', 'tempo', 'atraso', 'segundos'], preview: 'trailer', requires: 'heroTrailer',
  },
  {
    id: 'trailerQuality', section: 'trailers', group: 'Som e imagem',
    label: 'Qualidade do trailer',
    description: 'Limita a definição do vídeo do trailer. A fonte pode entregar uma resolução menor que a escolhida.',
    scope: 'profile', defaultValue: 'max',
    values: [
      choice('720', 'Até 720p', 'Pede trailers menores, quando a fonte permite.'),
      choice('1080', 'Até 1080p', 'Limita o trailer a Full HD.'),
      choice('max', 'Máxima disponível', 'Usa a maior definição disponível para o trailer.'),
    ],
    advanced: true, synonyms: ['trailer', 'resolução', 'definição', 'rede', 'internet'], preview: 'trailer',
  },
  {
    id: 'uiLang', section: 'languages', group: 'Cada idioma tem seu lugar',
    label: 'Idioma da interface',
    description: 'Muda menus e botões desta TV. Áudio, legendas e textos dos títulos continuam com suas próprias escolhas.',
    scope: 'device', defaultValue: 'auto',
    values: [
      choice('auto', 'Automático', 'No app, segue a conta e, sem ela, o idioma da TV.'),
      choice('pt', 'Português', 'Menus e botões em português.'),
      choice('en', 'English', 'Menus e botões em inglês.'),
      choice('es', 'Español', 'Menus e botões em espanhol.'),
    ],
    advanced: false, synonyms: ['língua', 'language', 'menu', 'português', 'inglês'], preview: 'language',
  },
  {
    id: 'audioLang', section: 'languages', group: 'Cada idioma tem seu lugar',
    label: 'Idioma preferido do áudio',
    description: 'Orienta a escolha da faixa quando o vídeo tem vários idiomas. Não cria dublagem se a faixa não existir.',
    scope: 'profile', defaultValue: 'original',
    values: [
      choice('original', 'Original', 'Não prioriza uma dublagem; usa a faixa original disponível.'),
      choice('pt', 'Português', 'Prefere áudio em português quando a faixa existe.'),
      choice('en', 'English', 'Prefere áudio em inglês quando a faixa existe.'),
      choice('es', 'Español', 'Prefere áudio em espanhol quando a faixa existe.'),
    ],
    advanced: false, synonyms: ['dublado', 'dublagem', 'voz', 'som', 'língua', 'audio'], preview: 'language',
  },
  {
    id: 'subtitleLang', section: 'languages', group: 'Cada idioma tem seu lugar',
    label: 'Idioma preferido das legendas',
    description: 'Escolhe o idioma de legenda preferido para as próximas reproduções. A faixa precisa estar disponível.',
    scope: 'profile', defaultValue: 'pt',
    values: [
      choice('account', 'Da conta', 'Segue a preferência de legenda do perfil, quando conhecida.'),
      choice('pt', 'Português', 'Prefere legendas em português.'),
      choice('en', 'English', 'Prefere legendas em inglês.'),
      choice('es', 'Español', 'Prefere legendas em espanhol.'),
    ],
    advanced: false, synonyms: ['legenda', 'legendado', 'subtitle', 'subtitles', 'cc', 'texto'], preview: 'language',
  },
  {
    id: 'metadataLang', section: 'languages', group: 'Cada idioma tem seu lugar',
    label: 'Idioma dos títulos e sinopses',
    description: 'Orienta os textos recebidos do TMDB. Um texto sem tradução pode continuar no idioma disponível; áudio e legendas não mudam.',
    scope: 'profile', defaultValue: 'interface',
    values: [
      choice('interface', 'Da interface', 'Pede textos no idioma da interface.'),
      choice('pt', 'Português', 'Pede títulos e sinopses em português, quando disponíveis.'),
      choice('en', 'English', 'Pede títulos e sinopses em inglês, quando disponíveis.'),
      choice('es', 'Español', 'Pede títulos e sinopses em espanhol, quando disponíveis.'),
    ],
    advanced: false, synonyms: ['metadados', 'tmdb', 'sinopse', 'nome', 'título', 'tradução'], preview: 'language',
  },
  {
    id: 'resolution', section: 'performance', group: 'Interface e imagens',
    label: 'Resolução da interface',
    description: 'Muda a definição de menus e botões. A qualidade do filme não muda; no app, a resolução depende da TV e pode exigir reinício.',
    scope: 'device', defaultValue: '1080',
    values: [
      choice('720', '720p · mais leve', 'Desenha menus em menor resolução; o texto pode ficar mais suave.'),
      choice('1080', '1080p · Full HD', 'Desenha a interface em Full HD.'),
      choice('2160', '2160p · 4K', 'Pede interface em 4K; a TV pode manter 1080p.'),
    ],
    advanced: false, synonyms: ['4k', '720p', '1080p', 'definição', 'lento', 'travando'], preview: 'performance',
  },
  {
    id: 'imageQuality', section: 'performance', group: 'Interface e imagens',
    label: 'Qualidade das imagens',
    description: 'Muda o tamanho das artes carregadas. Imagens menores pedem menos memória; não alteram a qualidade do vídeo.',
    scope: 'device', defaultValue: 'medium',
    values: [
      choice('low', 'Baixa', 'Carrega artes menores e usa menos memória.'),
      choice('medium', 'Média', 'Usa tamanho intermediário para cartazes e fundos.'),
      choice('high', 'Alta', 'Carrega artes maiores e usa mais memória.'),
    ],
    advanced: false, synonyms: ['cartaz', 'pôster', 'arte', 'ram', 'lento', 'travando'], preview: 'performance',
  },
  {
    id: 'glass', section: 'performance', group: 'Interface e imagens',
    label: 'Vidro nos painéis',
    description: 'Cria painéis translúcidos sobre o fundo. Desligado usa superfícies sólidas e reduz o trabalho de composição da imagem.',
    scope: 'device', defaultValue: 'on',
    values: toggle('Usa painéis translúcidos sobre a arte.', 'Usa painéis sólidos, com menos efeitos de composição.'),
    advanced: false, synonyms: ['transparência', 'desfoque', 'blur', 'lento', 'travando', 'gpu'], preview: 'performance',
  },
  {
    id: 'gpuEffects', section: 'performance', group: 'Interface e imagens',
    label: 'Efeitos da interface',
    description: 'Controla desfoque e brilho nas plataformas que oferecem esta opção. A amostra não mede a GPU de uma TV.',
    scope: 'device', defaultValue: 'auto',
    values: [
      choice('auto', 'Automático', 'No app, permite reduzir efeitos conforme a medição da TV.'),
      choice('light', 'Leves', 'Reduz desfoque e brilho.'),
      choice('full', 'Completos', 'Mantém os efeitos disponíveis na plataforma.'),
    ],
    advanced: false, synonyms: ['gpu', 'animação', 'brilho', 'desfoque', 'lento', 'travando'], preview: 'performance',
  },
  {
    id: 'rowLimit', section: 'performance', group: 'Carga da tela inicial',
    label: 'Limite de fileiras',
    description: 'Limita quantas fileiras a tela inicial monta. Menos fileiras também significam menos pedidos de catálogos e menos memória.',
    scope: 'device', defaultValue: '7',
    values: [
      choice('5', '5 fileiras', 'Monta até cinco fileiras na tela inicial.'),
      choice('7', '7 fileiras', 'Monta até sete fileiras na tela inicial.'),
      choice('12', '12 fileiras', 'Monta até doze fileiras, com maior uso de memória e rede.'),
      choice('16', '16 fileiras', 'Monta até dezesseis fileiras, com maior uso de memória e rede.'),
    ],
    advanced: false, synonyms: ['home', 'início', 'catálogo', 'linha', 'lento', 'travando'], preview: 'performance',
  },
  {
    id: 'textureMemory', section: 'performance', group: 'Carga da tela inicial',
    label: 'Memória para imagens',
    description: 'Define o teto do cache de imagens. Automático considera a RAM; no app, o valor solicitado pode ser limitado pelo aparelho.',
    scope: 'device', defaultValue: 'auto',
    values: [
      choice('auto', 'Automático', 'Deixa o app dimensionar o cache para o aparelho.'),
      choice('64', '64 MB', 'Solicita até 64 MB para o cache de imagens.'),
      choice('128', '128 MB', 'Solicita até 128 MB, respeitando o limite da TV.'),
      choice('256', '256 MB', 'Solicita até 256 MB, respeitando o limite da TV.'),
    ],
    advanced: true, synonyms: ['textura', 'cache', 'ram', 'megabytes', 'travando', 'memória'], preview: 'performance',
  },
]);

const optionsById = new Map(OPTIONS.map(option => [option.id, option]));

export const DEFAULTS = freeze(Object.fromEntries(OPTIONS.map(option => [option.id, option.defaultValue])));
export const DEMO_VALUES = freeze({ ...DEFAULTS, focusTrailer: 'on', uiLang: 'pt' });

export const RECOMMENDATION = freeze({
  label: 'Experimentar uma interface mais leve',
  description: 'Exemplo de conjunto para esta TV: painéis sólidos, imagens menores e até sete fileiras. Não é um diagnóstico do aparelho.',
  changes: { glass: 'off', imageQuality: 'low', rowLimit: '7' },
});

export function getOption(id) {
  const option = optionsById.get(id);
  if (!option) throw new RangeError(`Ajuste desconhecido: ${String(id)}`);
  return option;
}

export function valueLabel(id, value) {
  const option = getOption(id);
  const item = option.values.find(item => item.value === value);
  if (!item) throw new RangeError(`Valor inválido para ${id}: ${String(value)}`);
  return item.label;
}

const normalize = text => String(text ?? '').normalize('NFD').replace(/\p{M}/gu, '').toLowerCase().trim();

/** Empty searches are handled as local topic suggestions by the view. */
export function searchOptions(query) {
  const needle = normalize(query).replace(/\s+/g, ' ');
  if (!needle) return [];
  const words = needle.split(' ');
  return OPTIONS.map((option, index) => {
    const label = normalize(option.label);
    const synonyms = option.synonyms.map(normalize);
    const section = SECTIONS.find(section => section.id === option.section);
    const content = normalize([
      option.label, option.description, option.group, section.label,
      ...option.synonyms,
      ...option.values.flatMap(item => [item.label, item.description]),
    ].join(' '));
    if (!words.every(word => content.includes(word))) return null;
    const score = label === needle ? 1000
      : label.includes(needle) ? 800
      : synonyms.includes(needle) ? 600
      : synonyms.some(word => word.includes(needle)) ? 500
      : words.every(word => label.includes(word)) ? 400
      : 100;
    return { option, index, score };
  }).filter(Boolean).sort((a, b) => b.score - a.score || a.index - b.index).map(result => result.option);
}

export function changedOptions(values) {
  return OPTIONS.filter(option => (values[option.id] ?? option.defaultValue) !== option.defaultValue);
}

export function isAvailable(option, values) {
  const resolved = getOption(typeof option === 'string' ? option : option.id);
  if (resolved.requires && (values[resolved.requires] ?? DEFAULTS[resolved.requires]) !== 'on') {
    return {
      available: false,
      reason: `Ative “${getOption(resolved.requires).label}” para alterar esta opção.`,
      requires: resolved.requires,
    };
  }
  return { available: true, reason: '' };
}

function validate(changes) {
  if (!changes || typeof changes !== 'object' || Array.isArray(changes)) {
    throw new TypeError('As alterações devem ser um mapa de ajustes e valores.');
  }
  for (const [id, value] of Object.entries(changes)) {
    const option = getOption(id);
    if (typeof value !== 'string' || !option.values.some(item => item.value === value)) {
      throw new RangeError(`Valor inválido para ${id}: ${String(value)}`);
    }
  }
  return { ...changes };
}

/** Saved state changes only on commit or explicit demo reset. */
export function createModel(initial = DEMO_VALUES) {
  let saved = { ...DEFAULTS, ...validate(initial) };
  let draft = null;
  const copyDraft = () => draft ? { changes: { ...draft.changes }, previous: { ...draft.previous } } : null;

  return {
    get saved() { return { ...saved }; },
    get draft() { return copyDraft(); },
    begin(changes = {}) {
      const valid = validate(changes);
      draft = { changes: valid, previous: { ...saved } };
      return copyDraft();
    },
    candidate(id, value) {
      if (!draft) throw new Error('Abra uma edição antes de escolher um valor.');
      const valid = validate({ [id]: value });
      draft.changes = { ...draft.changes, ...valid };
      return copyDraft();
    },
    previewValues() {
      return { ...saved, ...(draft?.changes ?? {}) };
    },
    commit() {
      if (!draft) return [];
      const changed = OPTIONS.filter(option => Object.hasOwn(draft.changes, option.id)
        && saved[option.id] !== draft.changes[option.id]).map(option => option.id);
      saved = { ...saved, ...draft.changes };
      draft = null;
      return changed;
    },
    cancel() {
      draft = null;
      return { ...saved };
    },
    reset() {
      const changed = changedOptions(saved).map(option => option.id);
      saved = { ...DEFAULTS };
      draft = null;
      return changed;
    },
    snapshot() {
      return { saved: { ...saved }, draft: copyDraft() };
    },
  };
}
