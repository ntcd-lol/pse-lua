const fs = require('fs');
const path = require('path');
const matter = require('gray-matter');

const DOCS_DIR = process.env.DOCS_DIR || './docs';
const OUTPUT   = process.env.OUTPUT   || './manifest.json';
const PROJECT  = process.env.PROJECT  || 'pse-lua';
const OWNER    = process.env.OWNER    || 'ntcd-lol';

function parseInline(text) {
  const nodes = [];
  let i = 0;
  const push = (node) => { if (node) nodes.push(node); };

  while (i < text.length) {
    if (text[i] === '`') {
      const end = text.indexOf('`', i + 1);
      if (end !== -1) {
        push({ type: 'code', value: text.slice(i + 1, end) });
        i = end + 1;
        continue;
      }
    }
    if (text[i] === '*' && text[i + 1] === '*') {
      const end = text.indexOf('**', i + 2);
      if (end !== -1) {
        push({ type: 'bold', value: text.slice(i + 2, end) });
        i = end + 2;
        continue;
      }
    }
    if (text[i] === '*') {
      const end = text.indexOf('*', i + 1);
      if (end !== -1) {
        push({ type: 'italic', value: text.slice(i + 1, end) });
        i = end + 1;
        continue;
      }
    }
    if (text[i] === '[') {
      const close = text.indexOf(']', i + 1);
      if (close !== -1 && text[close + 1] === '(') {
        const paren = text.indexOf(')', close + 2);
        if (paren !== -1) {
          push({
            type: 'link',
            text: text.slice(i + 1, close),
            href: text.slice(close + 2, paren)
          });
          i = paren + 1;
          continue;
        }
      }
    }
    let j = i + 1;
    while (j < text.length && !'`*['.includes(text[j])) j++;
    push({ type: 'text', value: text.slice(i, j) });
    i = j;
  }

  const merged = [];
  for (const n of nodes) {
    if (n.type === 'text' && merged.length && merged[merged.length - 1].type === 'text') {
      merged[merged.length - 1].value += n.value;
    } else {
      merged.push(n);
    }
  }
  return merged;
}

function isTableRow(line) {
  return /^\s*\|.*\|\s*$/.test(line);
}
function isTableSeparator(line) {
  return /^\s*\|?\s*:?-{2,}:?\s*(\|\s*:?-{2,}:?\s*)+\|?\s*$/.test(line);
}
function splitTableRow(line) {
  let s = line.trim();
  if (s.startsWith('|')) s = s.slice(1);
  if (s.endsWith('|')) s = s.slice(0, -1);
  return s.split('|').map(c => c.trim());
}
function parseTable(lines, start) {
  const header = splitTableRow(lines[start]);
  const rows = [];
  let i = start + 1;
  if (i < lines.length && isTableSeparator(lines[i])) i++;
  while (i < lines.length && isTableRow(lines[i])) {
    rows.push(splitTableRow(lines[i]));
    i++;
  }
  return { block: { type: 'table', header, rows }, nextIndex: i };
}

function isListItem(line) {
  return /^\s*([-*+]|\d+\.)\s+/.test(line);
}
function parseList(lines, start) {
  const items = [];
  let i = start;
  let ordered = /^\s*\d+\./.test(lines[start]);
  while (i < lines.length && isListItem(lines[i])) {
    const m = lines[i].match(/^\s*([-*+]|\d+\.)\s+(.*)$/);
    items.push(m[2]);
    i++;
    while (i < lines.length && /^\s{2,}\S/.test(lines[i]) && !isListItem(lines[i])) {
      items[items.length - 1] += ' ' + lines[i].trim();
      i++;
    }
  }
  return { block: { type: 'list', ordered, items }, nextIndex: i };
}

function parseMarkdownToBlocks(markdown) {
  const lines = markdown.split('\n');
  const blocks = [];
  let i = 0;

  const flushParagraph = (buf) => {
    const text = buf.join('\n').trim();
    if (!text) return;
    blocks.push({ type: 'prose', inline: parseInline(text) });
  };

  let para = [];

  while (i < lines.length) {
    const line = lines[i];

    if (/^\s*</.test(line)) {
      flushParagraph(para); para = [];

      const htmlLines = [];
      let depth = 0;
      let started = false;

      while (i < lines.length) {
        const l = lines[i];
        htmlLines.push(l);

        const opens  = (l.match(/<(div|table|tr|td|th|thead|tbody|span|p|ul|ol|li|a|h[1-6])\b[^>]*>/gi) || []).length;
        const closes = (l.match(/<\/(div|table|tr|td|th|thead|tbody|span|p|ul|ol|li|a|h[1-6])\s*>/gi) || []).length;
        depth += opens - closes;
        if (opens > 0) started = true;

        i++;

        if (started && depth <= 0) break;
      }

      const html = htmlLines.join('\n').trim();
      if (html) blocks.push({ type: 'html', content: html });
      continue;
    }

    const fence = line.match(/^```(\w*)\s*$/);
    if (fence) {
      flushParagraph(para); para = [];
      const lang = fence[1] || 'text';
      const codeLines = [];
      i++;
      while (i < lines.length && !/^```\s*$/.test(lines[i])) {
        codeLines.push(lines[i]);
        i++;
      }
      i++;
      blocks.push({ type: 'code', language: lang, content: codeLines.join('\n') });
      continue;
    }

    const h = line.match(/^(#{1,6})\s+(.+?)\s*$/);
    if (h) {
      flushParagraph(para); para = [];
      blocks.push({ type: 'heading', level: h[1].length, inline: parseInline(h[2]) });
      i++;
      continue;
    }

    if (isTableRow(line) && i + 1 < lines.length && isTableSeparator(lines[i + 1])) {
      flushParagraph(para); para = [];
      const { block, nextIndex } = parseTable(lines, i);
      block.header = block.header.map(parseInline);
      block.rows   = block.rows.map(r => r.map(parseInline));
      blocks.push(block);
      i = nextIndex;
      continue;
    }

    if (isListItem(line)) {
      flushParagraph(para); para = [];
      const { block, nextIndex } = parseList(lines, i);
      block.items = block.items.map(parseInline);
      blocks.push(block);
      i = nextIndex;
      continue;
    }

    if (/^\s*---+\s*$/.test(line)) {
      flushParagraph(para); para = [];
      blocks.push({ type: 'hr' });
      i++;
      continue;
    }

    if (line.trim() === '') {
      flushParagraph(para); para = [];
      i++;
      continue;
    }

    para.push(line);
    i++;
  }
  flushParagraph(para);
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
