import { compareProbe } from './compare.mjs';

const status = document.getElementById('status');
const output = document.getElementById('result');

async function readJson(url) {
  const response = await fetch(url, { cache: 'no-store' });
  if (!response.ok) throw new Error(`${url}: HTTP ${response.status}`);
  return response.json();
}

function drawTerrain(samples) {
  const canvas = document.getElementById('viewport');
  const gl = canvas.getContext('webgl2');
  if (!gl) throw new Error('WebGL 2 indisponível');
  const side = Math.sqrt(samples.length);
  if (!Number.isInteger(side) || side < 2) throw new Error('Invalid terrain grid');
  const vertices = [];
  const add = (x, z) => vertices.push(x / (side - 1) * 2 - 1,
    samples[z * side + x] * 0.4 - 0.2, z / (side - 1) * 2 - 1);
  for (let z = 0; z < side - 1; ++z) {
    for (let x = 0; x < side - 1; ++x) {
      add(x, z); add(x + 1, z); add(x, z + 1);
      add(x + 1, z); add(x + 1, z + 1); add(x, z + 1);
    }
  }
  const shader = (type, source) => {
    const result = gl.createShader(type);
    gl.shaderSource(result, source);
    gl.compileShader(result);
    if (!gl.getShaderParameter(result, gl.COMPILE_STATUS)) throw new Error(gl.getShaderInfoLog(result));
    return result;
  };
  const vertex = shader(gl.VERTEX_SHADER, `#version 300 es
    in vec3 position;
    uniform float angle;
    uniform float aspect;
    out float height;
    void main() {
      float c = cos(angle), s = sin(angle);
      vec3 p = vec3(c*position.x + s*position.z, position.y,
                    -s*position.x + c*position.z);
      float y = 0.75*p.y - 0.66*p.z;
      float z = 0.66*p.y + 0.75*p.z;
      gl_Position = vec4(p.x*0.65/max(aspect,1.0), y*0.65*min(aspect,1.0), z*0.4, 1.0);
      height = (position.y + 0.2) / 0.4;
    }`);
  const fragment = shader(gl.FRAGMENT_SHADER, `#version 300 es
    precision highp float;
    in float height;
    out vec4 color;
    void main() { color = vec4(mix(vec3(0.06,0.25,0.5), vec3(0.4,0.95,0.65),height),1.0); }`);
  const program = gl.createProgram();
  gl.attachShader(program, vertex);
  gl.attachShader(program, fragment);
  gl.linkProgram(program);
  gl.deleteShader(vertex);
  gl.deleteShader(fragment);
  if (!gl.getProgramParameter(program, gl.LINK_STATUS)) throw new Error(gl.getProgramInfoLog(program));
  gl.useProgram(program);
  const vao = gl.createVertexArray();
  gl.bindVertexArray(vao);
  const buffer = gl.createBuffer();
  gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
  gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(vertices), gl.STATIC_DRAW);
  const location = gl.getAttribLocation(program, 'position');
  gl.enableVertexAttribArray(location);
  gl.vertexAttribPointer(location, 3, gl.FLOAT, false, 0, 0);
  const angle = gl.getUniformLocation(program, 'angle');
  const aspect = gl.getUniformLocation(program, 'aspect');
  gl.enable(gl.DEPTH_TEST);
  gl.clearColor(0.03, 0.07, 0.12, 1);
  const frame = (time) => {
    const scale = Math.min(devicePixelRatio || 1, 2);
    const width = Math.max(1, Math.round(canvas.clientWidth * scale));
    const height = Math.max(1, Math.round(canvas.clientHeight * scale));
    if (canvas.width !== width || canvas.height !== height) {
      canvas.width = width;
      canvas.height = height;
    }
    gl.viewport(0, 0, canvas.width, canvas.height);
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
    gl.uniform1f(angle, time * 0.00015);
    gl.uniform1f(aspect, canvas.width / canvas.height);
    gl.drawArrays(gl.TRIANGLES, 0, vertices.length / 3);
  };
  frame(0);
  const pixel = new Uint8Array(4);
  gl.readPixels(Math.floor(canvas.width / 2), Math.floor(canvas.height / 2),
    1, 1, gl.RGBA, gl.UNSIGNED_BYTE, pixel);
  if (gl.getError() !== gl.NO_ERROR || pixel[1] < 40) {
    throw new Error('WebGL draw/readback failed: no terrain at the center');
  }
  const animate = (time) => { frame(time); requestAnimationFrame(animate); };
  requestAnimationFrame(animate);
  canvas.addEventListener('webglcontextlost', () => {
    status.dataset.state = 'FAIL';
    status.textContent = 'FAIL — contexto WebGL perdido; recarregue a prova.';
  });
  return { triangles: vertices.length / 9, centerPixel: [...pixel], webgl: gl.getParameter(gl.VERSION) };
}

try {
  const [native, manifest] = await Promise.all([readJson('./native.json'), readJson('./manifest.json')]);
  const { default: createModule } = await import('./probe.mjs');
  const lines = [];
  await createModule({ print: line => lines.push(line), printErr: line => console.error(line) });
  const wasm = JSON.parse(lines.join('\n'));
  const compared = compareProbe(native, wasm);
  if (wasm.abi.longBytes !== 4 || wasm.abi.pointerBytes !== 4) throw new Error('Unexpected wasm32 ABI');
  const terrain = drawTerrain(wasm.cases.find(value => value.seed === 42).uniform);
  output.textContent = JSON.stringify({ status: 'PASS', compared, nativeABI: native.abi,
    wasmABI: wasm.abi, ...terrain, manifest }, null, 2);
  status.dataset.state = 'PASS';
  status.textContent = `PASS — ${compared.uniform} valores uniformes idênticos; ` +
    `${compared.gaussian} gaussianos dentro de 1e-12 relativo; WebGL 2 desenhou a malha.`;
} catch (error) {
  status.dataset.state = 'FAIL';
  status.textContent = `FAIL — ${error.message}`;
  output.textContent = JSON.stringify({ status: 'FAIL', error: error.message }, null, 2);
  console.error(error);
}
