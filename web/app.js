// PW Web: independent WebGL 2 / WASM diagnostic. No Perfect World code/assets.
const byId = id => document.getElementById(id);
const canvas = byId("viewport");
const gl = canvas.getContext("webgl2", { antialias: true, alpha: false });
byId("webgl").textContent = gl ? "WebGL 2 disponível" : "WebGL 2 indisponível";
byId("gpu").textContent = navigator.gpu ? "WebGPU detectado" : "WebGPU não detectado";
byId("graphics-state").textContent = gl ? "ATIVO" : "INDISPONÍVEL";

if (gl) {
  const compile = (kind, source) => {
    const shader = gl.createShader(kind);
    gl.shaderSource(shader, source);
    gl.compileShader(shader);
    if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
      const message = gl.getShaderInfoLog(shader);
      gl.deleteShader(shader);
      throw new Error("Shader compile failed: " + message);
    }
    return shader;
  };
  try {
    const vs = compile(gl.VERTEX_SHADER, `#version 300 es
      in vec2 a_position;
      uniform float u_time;
      void main() {
        float c = cos(u_time), s = sin(u_time);
        vec2 p = vec2(a_position.x*c - a_position.y*s,
                      a_position.x*s + a_position.y*c);
        gl_Position = vec4(p, 0.0, 1.0);
      }`);
    const fs = compile(gl.FRAGMENT_SHADER, `#version 300 es
      precision highp float;
      out vec4 color;
      void main() { color = vec4(0.28, 0.9, 0.78, 1.0); }`);
    const program = gl.createProgram();
    gl.attachShader(program, vs); gl.attachShader(program, fs);
    gl.linkProgram(program);
    gl.deleteShader(vs); gl.deleteShader(fs);
    if (!gl.getProgramParameter(program, gl.LINK_STATUS)) throw new Error(gl.getProgramInfoLog(program));
    const attribute = gl.getAttribLocation(program, "a_position");
    const timeUniform = gl.getUniformLocation(program, "u_time");
    const vao = gl.createVertexArray();
    const buffer = gl.createBuffer();
    gl.bindVertexArray(vao);
    gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array([0,0.62,-0.62,-0.48,0.62,-0.48]), gl.STATIC_DRAW);
    gl.enableVertexAttribArray(attribute);
    gl.vertexAttribPointer(attribute, 2, gl.FLOAT, false, 0, 0);
    gl.bindVertexArray(null);

    let frames = 0, lastFps = performance.now();
    function render(now) {
      const scale = Math.min(devicePixelRatio || 1, 2);
      const w = Math.round(canvas.clientWidth * scale);
      const h = Math.round(canvas.clientHeight * scale);
      if (canvas.width !== w || canvas.height !== h) { canvas.width = w; canvas.height = h; }
      gl.viewport(0, 0, canvas.width, canvas.height);
      gl.clearColor(0.03, 0.10, 0.16, 1);
      gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
      gl.useProgram(program);
      gl.uniform1f(timeUniform, now * 0.00065);
      gl.bindVertexArray(vao);
      gl.drawArrays(gl.TRIANGLES, 0, 3);
      gl.bindVertexArray(null);
      frames++;
      if (now - lastFps >= 1000) {
        byId("fps").textContent = String(Math.round(frames * 1000 / (now - lastFps))) + " FPS";
        frames = 0; lastFps = now;
      }
      requestAnimationFrame(render);
    }
    requestAnimationFrame(render);
  } catch (error) {
    console.error(error);
    byId("graphics-state").textContent = "ERRO DE SHADER";
  }
}

// The optional WASM module is generated locally by ./scripts/build-wasm.sh.
// Absence is expected before running emcc and is not a failed port.
try {
  const response = await fetch("/wasm/pw_core.mjs", { method: "HEAD" });
  if (!response.ok) throw new Error("not built");
  const { default: createPwCore } = await import("/wasm/pw_core.mjs");
  const module = await createPwCore();
  const version = module._pw_core_version();
  const sum = module._pw_core_add(7, 5);
  if (version !== 1 || sum !== 12) throw new Error("C++ bridge self-test failed");
  byId("wasm").textContent = "WebAssembly carregado";
  byId("wasm-detail").textContent = "C++ core v" + version + " · 7 + 5 = " + sum;
} catch (error) {
  byId("wasm").textContent = error.message === "not built" ? "Aguardando build" : "Falha na carga";
  byId("wasm-detail").textContent = error.message === "not built" ?
    "Execute ./scripts/build-wasm.sh" : String(error.message);
}
