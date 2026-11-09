const fs = require('fs');
const path = require('path');
const matter = require('gray-matter');

const DOCS_DIR = process.env.DOCS_DIR || './docs';
const OUTPUT   = process.env.OUTPUT   || './manifest.json';
const PROJECT  = process.env.PROJECT  || 'pse-lua';
const OWNER    = process.env.OWNER    || 'ntcd-lol';
const BRANCH   = process.env.BRANCH   || 'main';
const RAW_BASE = process.env.RAW_BASE ||
  `https://raw.githubusercontent.com/${OWNER}/${PROJECT}/${BRANCH}`;

function slugify(text) {
  const map = {
    'а':'a','б':'b','в':'v','г':'g','д':'d','е':'e','ё':'yo','ж':'zh',
    'з':'z','и':'i','й':'y','к':'k','л':'l','м':'m','н':'n','о':'o',
    'п':'p','р':'r','с':'s','т':'t','у':'u','ф':'f','х':'h','ц':'ts',
    'ч':'ch','ш':'sh','щ':'sch','ъ':'','ы':'y','ь':'','э':'e','ю':'yu',
    'я':'ya'
  };

  return String(text || '')
    .toLowerCase()
    .split('')
    .map(ch => map[ch] !== undefined ? map[ch] : ch)
    .join('')
    .replace(/[`*_~\[\](){}<>]/g, '')
    .replace(/[^a-z0-9\s-]/g, '')
    .trim()
    .replace(/\s+/g, '-')
    .replace(/-+/g, '-')
    .replace(/^-|-$/g, '') || 'section';
}

function parseInline(text) {
  const nodes = [];
  let i = 0;

  const pushText = (value) => {
    if (!value) return;
    const last = nodes[nodes.length - 1];
    if (last && last.type === 'text') last.value += value;
    else nodes.push({ type: 'text', value });
  };

  while (i < text.length) {
    const ch = text[i];

    if (ch === '`') {
      const end = text.indexOf('`', i + 1);
      if (end !== -1) {
        nodes.push({ type: 'code', value: text.slice(i + 1, end) });
        i = end + 1;
        continue;
      }
    }

    if (ch === '*' && text[i + 1] === '*') {
      const end = text.indexOf('**', i + 2);
      if (end !== -1) {
        nodes.push({ type: 'bold', value: text.slice(i + 2, end) });
        i = end + 2;
        continue;
      }
    }

    if (ch === '*') {
      const end = text.indexOf('*', i + 1);
      if (end !== -1) {
        nodes.push({ type: 'italic', value: text.slice(i + 1, end) });
        i = end + 1;
        continue;
      }
    }

    if (ch === '!' && text[i + 1] === '[') {
      const close = text.indexOf(']', i + 2);
      if (close !== -1 && text[close + 1] === '(') {
        const paren = text.indexOf(')', close + 2);
        if (paren !== -1) {
          nodes.push({
            type: 'image',
            alt: text.slice(i + 2, close),
            src: text.slice(close + 2, paren)
          });
          i = paren + 1;
          continue;
        }
      }
    }

    if (ch === '[') {
      const close = text.indexOf(']', i + 1);
      if (close !== -1 && text[close + 1] === '(') {
        const paren = text.indexOf(')', close + 2);
        if (paren !== -1) {
          nodes.push({
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
    while (j < text.length && !'`*!['.includes(text[j])) j++;
    pushText(text.slice(i, j));
    i = j;
  }

  return nodes;
}

function stripHtmlComments(md) {
  return md.replace(/<!--[\s\S]*?-->/g, '');
}

function rewriteAssetUrls(markdown, fileDir) {
  const joinBase = (p) => {
    if (!p) return p;
    if (p.startsWith('#')) return p;
    if (p.startsWith('//')) return p;
    if (/^https?:\/\//i.test(p)) return p;
    if (/\.md(?:#.*)?$/i.test(p)) return p;
    if (p.startsWith('/')) return RAW_BASE + p;
    const clean = p.replace(/^\.\//, '');
    const prefix = fileDir ? `${fileDir}/` : '';
    return `${RAW_BASE}/${prefix}${clean}`;
  };

  let s = markdown;

  s = s.replace(/!\[([^\]]*)\]\(([^)\s]+)\)/g, (_, alt, p) => `![${alt}](${joinBase(p)})`);

  s = s.replace(/(?<!!)\[([^\]]*)\]\(([^)\s]+)\)/g, (_, text, p) => {
    if (p.startsWith('#') || p.startsWith('//') || /^https?:\/\//i.test(p) || /\.md(?:#.*)?$/i.test(p)) {
      return `[${text}](${p})`;
    }
    return `[${text}](${joinBase(p)})`;
  });

  s = s.replace(/\bsrc=(["'])([^"']+)\1/g, (_, q, p) => `src=${q}${joinBase(p)}${q}`);
  s = s.replace(/\bhref=(["'])([^"']+)\1/g, (_, q, p) => `href=${q}${joinBase(p)}${q}`);

  return s;
}

function extractHeadingFromHeader(content) {
  const head = content.slice(0, 2000);
  const m = head.match(/<h[1-6][^>]*>([\s\S]*?)<\/h[1-6]>/i);
  if (!m) return null;

  let text = m[1]
    .replace(/<[^>]+>/g, ' ')
    .replace(/&nbsp;/g, ' ')
    .replace(/&amp;/g, '&')
    .replace(/&lt;/g, '<')
    .replace(/&gt;/g, '>')
    .replace(/\s+/g, ' ')
    .trim();

  text = text.replace(/\s*-+>\s*/g, ' → ');

  const arrowIdx = text.lastIndexOf('→');
  if (arrowIdx !== -1) text = text.slice(arrowIdx + 1).trim();

  text = text.replace(/^Документация\s*[:\-–—]?\s*/i, '').trim();

  return text || null;
}

const CALLOUT_VARIANTS = new Set(['note', 'tip', 'important', 'warning', 'caution', 'danger', 'info']);

function isCalloutStart(line) {
  return /^\s*>\s*\[!(\w+)\]\s*$/i.test(line);
}

function parseCallout(lines, start) {
  const m = lines[start].match(/^\s*>\s*\[!(\w+)\]\s*$/i);
  const variant = (m[1] || 'note').toLowerCase();
  const safeVariant = CALLOUT_VARIANTS.has(variant) ? variant : 'note';

  const bodyLines = [];
  let i = start + 1;

  while (i < lines.length) {
    const l = lines[i];

    if (isCalloutStart(l)) break;

    if (l.trim() === '') {
      if (i + 1 < lines.length && /^\s*>/.test(lines[i + 1]) && !isCalloutStart(lines[i + 1])) {
        bodyLines.push('');
        i++;
        continue;
      }
      break;
    }

    const q = l.match(/^\s*>\s?(.*)$/);
    if (!q) break;

    bodyLines.push(q[1]);
    i++;
  }

  const text = bodyLines.join('\n').trim();

  return {
    block: { type: 'callout', variant: safeVariant, inline: parseInline(text) },
    nextIndex: i
  };
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
  const ordered = /^\s*\d+\./.test(lines[start]);
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

const HTML_PAIR_TAGS =
  'div|table|thead|tbody|tr|td|th|span|p|ul|ol|li|a|h[1-6]|' +
  'section|article|header|footer|nav|main|figure|figcaption|' +
  'details|summary|strong|em|b|i|code|pre|blockquote';

const HTML_OPEN_RE  = new RegExp(`<(${HTML_PAIR_TAGS})\\b[^>]*>`, 'gi');
const HTML_CLOSE_RE = new RegExp(`</(${HTML_PAIR_TAGS})\\s*>`, 'gi');

function isHtmlStart(line) {
  return /^\s*<(!|div|table|section|article|header|footer|nav|main|p|ul|ol|h[1-6]|span|figure|img|br|hr)\b/i.test(line);
}

function parseHtmlBlock(lines, start) {
  const htmlLines = [];
  let depth = 0;
  let started = false;
  let i = start;

  while (i < lines.length) {
    const l = lines[i];
    htmlLines.push(l);

    const opens  = (l.match(HTML_OPEN_RE)  || []).length;
    const closes = (l.match(HTML_CLOSE_RE) || []).length;
    depth += opens - closes;
    if (opens > 0) started = true;

    i++;

    if (started && depth <= 0) break;
  }

  return {
    block: { type: 'html', content: htmlLines.join('\n').trim() },
    nextIndex: i
  };
}

function parseMarkdownToBlocks(markdown) {
  const lines = markdown.split('\n');
  const blocks = [];
  let i = 0;
  let para = [];

  const flushParagraph = () => {
    const text = para.join('\n').trim();
    para = [];
    if (!text) return;
    blocks.push({ type: 'prose', inline: parseInline(text) });
  };

  while (i < lines.length) {
    const line = lines[i];

    if (isCalloutStart(line)) {
      flushParagraph();
      const { block, nextIndex } = parseCallout(lines, i);
      blocks.push(block);
      i = nextIndex;
      continue;
    }

    if (isHtmlStart(line)) {
      flushParagraph();
      const { block, nextIndex } = parseHtmlBlock(lines, i);
      blocks.push(block);
      i = nextIndex;
      continue;
    }

    const fence = line.match(/^```(\w*)\s*$/);
    if (fence) {
      flushParagraph();
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
      flushParagraph();
      blocks.push({ type: 'heading', level: h[1].length, inline: parseInline(h[2]) });
      i++;
      continue;
    }

    if (isTableRow(line) && i + 1 < lines.length && isTableSeparator(lines[i + 1])) {
      flushParagraph();
      const { block, nextIndex } = parseTable(lines, i);
      block.header = block.header.map(parseInline);
      block.rows   = block.rows.map(r => r.map(parseInline));
      blocks.push(block);
      i = nextIndex;
      continue;
    }

    if (isListItem(line)) {
      flushParagraph();
      const { block, nextIndex } = parseList(lines, i);
      block.items = block.items.map(parseInline);
      blocks.push(block);
      i = nextIndex;
      continue;
    }

    if (/^\s*---+\s*$/.test(line)) {
      flushParagraph();
      blocks.push({ type: 'hr' });
      i++;
      continue;
    }

    if (line.trim() === '') {
      flushParagraph();
      i++;
      continue;
    }

    para.push(line);
    i++;
  }
  flushParagraph();

  return blocks;
}

function mdPathToSectionId(href, sections, currentSectionId) {
  if (!href) return href;
  if (/^https?:\/\//i.test(href)) return href;

  let pathPart = href;
  let anchor = '';
  const hashIdx = pathPart.indexOf('#');
  if (hashIdx !== -1) {
    anchor = decodeURIComponent(pathPart.slice(hashIdx + 1));
    pathPart = pathPart.slice(0, hashIdx);
  }

  const anchorSlug = anchor ? slugify(anchor) : '';

  if (!pathPart) {
    if (!currentSectionId || !anchorSlug) return href;
    return `#sec-${currentSectionId}-${anchorSlug}`;
  }

  if (!/\.md$/i.test(pathPart)) return href;

  let clean = pathPart
    .replace(/^\.\//, '')
    .replace(/^\/+/, '')
    .replace(/\.md$/i, '');

  clean = clean.replace(/^docs\//, '');

  const id = clean.split('/').join('-');

  const exists = sections.some(s => s.id === id);
  if (!exists) return href;

  if (anchorSlug) return `#sec-${id}-${anchorSlug}`;
  return `#sec-${id}`;
}

function rewriteMdLinksInBlocks(blocks, sections, currentSectionId) {
  const walkInline = (nodes) => {
    if (!Array.isArray(nodes)) return;
    for (const n of nodes) {
      if (!n) continue;
      if (n.type === 'link' && typeof n.href === 'string') {
        n.href = mdPathToSectionId(n.href, sections, currentSectionId);
      }
    }
  };

  for (const b of blocks || []) {
    if (!b || typeof b !== 'object') continue;

    switch (b.type) {
      case 'heading':
      case 'prose':
      case 'callout':
        walkInline(b.inline);
        break;

      case 'list':
        for (const item of b.items || []) walkInline(item);
        break;

      case 'table':
        for (const cell of b.header || []) walkInline(cell);
        for (const row of b.rows || []) for (const cell of row) walkInline(cell);
        break;

      case 'html':
        if (typeof b.content === 'string') {
          b.content = b.content.replace(
            /\bhref=(["'])([^"']+\.md(?:#[^"']*)?)\1/gi,
            (_, q, p) => {
              const replaced = mdPathToSectionId(p, sections, currentSectionId);
              return `href=${q}${replaced}${q}`;
            }
          );
        }
        break;
    }
  }
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

function collectDocs() {
  const files = collectMarkdownFiles(DOCS_DIR);
  const rootReadme = path.resolve('README.md');
  const hasRootReadme = files.some(f => path.resolve(f) === rootReadme);
  if (!hasRootReadme && fs.existsSync(rootReadme)) {
    files.unshift(rootReadme);
  }
  return files;
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

    const order = typeof data.order === 'number'
      ? data.order
      : (file === path.resolve('README.md') ? -1 : index);

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

buildManifest();
