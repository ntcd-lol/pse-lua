const fs = require('fs');
const path = require('path');
const matter = require('gray-matter');

const DOCS_DIR = process.env.DOCS_DIR || './docs';
const OUTPUT   = process.env.OUTPUT   || './manifest.json';
const PROJECT  = process.env.PROJECT  || 'pse-lua';
const OWNER    = process.env.OWNER    || 'ntcd-lol';

function parseMarkdownToBlocks(markdown) {
  const lines = markdown.split('\n');
  const blocks = [];

  let buf = [];
  let mode = 'prose';
  let lang = null;

  const flushProse = () => {
    const text = buf.join('\n').trim();
    buf = [];
    if (!text) return;

    // Разбиваем prose на подблоки по заголовкам
    const segments = text.split(/\n(?=#{1,6}\s)/);
    for (const seg of segments) {
      const trimmed = seg.trim();
      if (!trimmed) continue;

      const headingMatch = trimmed.match(/^(#{1,6})\s+(.+)$/s);
      if (headingMatch) {
        blocks.push({
          type: 'heading',
          level: headingMatch[1].length,
          content: headingMatch[2].trim()
        });
      } else {
        blocks.push({ type: 'prose', content: trimmed });
      }
    }
  };

  const flushCode = () => {
    const text = buf.join('\n');
    buf = [];
    blocks.push({ type: 'code', language: lang || 'text', content: text });
  };

  for (const line of lines) {
    const fence = line.match(/^```(\w*)\s*$/);
    if (fence) {
      if (mode === 'prose') {
        flushProse();
        mode = 'code';
        lang = fence[1] || 'text';
      } else {
        flushCode();
        mode = 'prose';
        lang = null;
      }
      continue;
    }
    buf.push(line);
  }

  if (mode === 'code') flushCode();
  else flushProse();

  return blocks;
}

function collectMarkdownFiles(dir) {
  const out = [];
  const walk = (current) => {
    for (const entry of fs.readdirSync(current, { withFileTypes: true })) {
      const full = path.join(current, entry.name);
      if (entry.isDirectory()) walk(full);
      else if (entry.isFile() && entry.name.endsWith('.md')) out.push(full);
    }
  };
  if (fs.existsSync(dir)) walk(dir);
  return out;
}

function buildManifest() {
  const files = collectMarkdownFiles(DOCS_DIR);

  const sections = files.map((file, index) => {
    const rel = path.relative(DOCS_DIR, file).replace(/\\/g, '/');
    const raw = fs.readFileSync(file, 'utf8');
    const { data, content } = matter(raw);

    const id = data.id || rel.replace(/\.md$/, '').replace(/\//g, '-');
    return {
      id,
      title: data.title || id,
      sourceFile: rel,
      order: typeof data.order === 'number' ? data.order : index,
      blocks: parseMarkdownToBlocks(content)
    };
  });

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
}

buildManifest();
