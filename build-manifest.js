function computeOrder(rel, data, index, isRootReadme) {
  if (typeof data.order === 'number') return data.order;
  if (isRootReadme) return -1000;

  const noExt = rel.endsWith('.md') ? rel.slice(0, -3) : rel;
  const parts = noExt.split('/');

  let weight = parts.length * 1000;

  const last = parts[parts.length - 1];
  if (last === 'index') weight -= 500;

  const alphaKey = noExt.replace(/\//g, '-');
  let alphaNum = 0;
  for (let i = 0; i < Math.min(alphaKey.length, 6); i++) {
    alphaNum = alphaNum * 36 + (parseInt(alphaKey[i], 36) || 0);
  }

  return weight + (alphaNum % 1000);
}

function buildManifest() {
  const files = collectDocs();

  const sections = files.map((file, index) => {
    let rel = path.relative(DOCS_DIR, file).split(path.sep).join('/');
    if (rel.startsWith('..')) rel = path.basename(file);

    const raw = fs.readFileSync(file, 'utf8');
    const { data, content } = matter(raw);

    const noExt = rel.endsWith('.md') ? rel.slice(0, -3) : rel;
    const id    = data.id || noExt.split('/').join('-');

    const fromHeader = extractHeadingFromHeader(content);
    const title = data.title || fromHeader || id;

    const isRootReadme = file === path.resolve('README.md');
    const order = computeOrder(rel, data, index, isRootReadme);

    const relDir = path.dirname(rel).split(path.sep).join('/');
    const fileDir = relDir === '.' ? '' : relDir;

    const withoutComments = stripHtmlComments(content);
    const rewritten = rewriteAssetUrls(withoutComments, fileDir);
    const blocks = parseMarkdownToBlocks(rewritten);

    return {
      id,
      title,
      sourceFile: rel,
      order,
      blocks
    };
  });

  for (const s of sections) {
    rewriteMdLinksInBlocks(s.blocks, sections, s.id);
  }

  sections.sort((a, b) => a.order - b.order || a.id.localeCompare(b.id));
  sections.forEach(s => delete s.order);

  const manifest = {
    project: PROJECT,
    owner: OWNER,
    generatedAt: new Date().toISOString(),
    sections
  };

  fs.writeFileSync(OUTPUT, JSON.stringify(manifest, null, 2) + '\n', 'utf8');
  console.log(`✓ ${OUTPUT} — ${sections.length} sections from ${DOCS_DIR}`);
  console.log(`  RAW_BASE = ${RAW_BASE}`);
  sections.forEach(s => console.log(`  • ${s.sourceFile} → "${s.title}"`));
}
