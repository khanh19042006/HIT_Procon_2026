const seed = {
  config: {
    startsAt: 1778227200, daySeconds: [5, 5], daySteps: [10, 10],
    map: { height: 4, width: 4, cells: [[0, 0, 1, 2], [0, 3, 1, 0], [1, 0, 0, 0], [2, 0, 0, 0]] },
    spots: [{ brand: 0, pos: 1, stocks: 4 }], agents: [0, 5], fuelLimits: 20, players: 2, busyThreshold: 2, jammedThreshold: 4
  },
  states: [
    { endsAt: 1778227205, day: 0, agents: [{ kind: 0, pos: 0, fuel: 20 }, { kind: 0, pos: 5, fuel: 20 }], others: [], traffics: [] },
    { endsAt: 1778227210, day: 1, agents: [{ kind: 0, pos: 1, fuel: 19 }, { kind: 0, pos: 5, fuel: 20 }], others: [], traffics: [] }
  ]
};

let data = structuredClone(seed);
let dayIndex = 0;
let step = 0;
let selectedAgent = 0;
let playing = false;
let timer;
let agents = [];
let events = [];
let completedDays = new Set();
let consoleLines = [];
let plannedActions = null;
let advancing = false;
const difficultyProfiles = {
  easy: { width: 10, height: 10, fuel: 20, agents: 4, spots: 6, players: 1, busy: 2, jam: 4 },
  medium: { width: 12, height: 12, fuel: 18, agents: 6, spots: 8, players: 2, busy: 2, jam: 4 },
  hard: { width: 16, height: 12, fuel: 15, agents: 8, spots: 10, players: 3, busy: 1, jam: 3 }
};
const $ = (id) => document.getElementById(id);

function currentState() { return data.states[Math.min(dayIndex, data.states.length - 1)]; }
function posOf(pos) { return { x: pos % data.config.map.width, y: Math.floor(pos / data.config.map.width) }; }
function posId(x, y) { return y * data.config.map.width + x; }
function nextPosition(pos, direction) {
  const point = posOf(pos);
  const even = [[-1,-1],[0,-1],[1,0],[0,1],[-1,1],[-1,0]];
  const odd = [[0,-1],[1,-1],[1,0],[1,1],[0,1],[-1,0]];
  const delta = (point.y % 2 ? odd : even)[direction];
  return { x: point.x + delta[0], y: point.y + delta[1] };
}
function isWalkable(point) { return point.x >= 0 && point.x < data.config.map.width && point.y >= 0 && point.y < data.config.map.height && data.config.map.cells[point.y][point.x] !== 3; }
function validDirections(pos) { return [0, 1, 2, 3, 4, 5].filter(direction => isWalkable(nextPosition(pos, direction))); }
function randomize(items) { return items.sort(() => Math.random() - 0.5); }
function terrainName(type) { return ['Flat ground', 'Road', 'Mountain', 'Pond'][type] || 'Unknown'; }
function addEvent(message, tone = '') { events.unshift({ message, tone, time: `DAY ${currentState().day} · STEP ${step}` }); events = events.slice(0, 8); renderEvents(); }
function appendConsole(message) { consoleLines.push(`[DAY ${currentState().day} · STEP ${step}] ${message}`); $('day-console').textContent = consoleLines.join('\n'); }
function setDay(index, resetConsole = false) { dayIndex = Math.max(0, Math.min(index, data.states.length - 1)); step = 0; plannedActions = null; agents = structuredClone(currentState().agents); if (resetConsole) { consoleLines = []; appendConsole(`Loaded state with ${agents.length} agents`); } render(); }
function render() { renderStatus(); renderAgents(); renderMap(); renderInspector(); renderEvents(); renderRunDays(); renderDaySettings(); renderConsole(); }
function renderStatus() {
  $('day-value').textContent = `${currentState().day + 1} / ${data.config.daySteps.length}`;
  $('step-value').textContent = `${step} / ${data.config.daySteps[currentState().day] || 0}`;
  $('deadline-value').textContent = currentState().endsAt;
  $('map-size').textContent = `${data.config.map.width} x ${data.config.map.height}`;
  $('agent-count').textContent = agents.length;
  $('run-status').textContent = playing ? 'Playing' : 'Ready';
  $('play-button').textContent = playing ? 'Pause simulation' : 'Play simulation';
}
function renderConsole() { $('day-console').textContent = consoleLines.join('\n') || 'Waiting for a day run...'; }
function renderRunDays() {
  $('completed-days').textContent = `${completedDays.size}/${data.config.daySteps.length}`;
  $('run-days').innerHTML = data.config.daySteps.map((steps, index) => `<button class="run-day ${completedDays.has(index) ? 'completed' : ''}" data-run-day="${index}" ${index > 0 && !completedDays.has(index - 1) ? 'disabled' : ''}><span>D${index + 1}</span><small>${steps} steps</small><b>${completedDays.has(index) ? 'DONE' : 'RUN'}</b></button>`).join('');
  document.querySelectorAll('[data-run-day]').forEach(button => button.addEventListener('click', () => runDay(Number(button.dataset.runDay))));
}
function renderDaySettings() {
  $('days-label').textContent = `${data.config.daySteps.length} days`;
  $('day-settings-list').innerHTML = data.config.daySteps.map((steps, index) => `<div class="day-input"><span>D${index + 1}</span><label>steps<input data-day-steps="${index}" type="number" min="1" value="${steps}" aria-label="Day ${index + 1} steps"></label><label>sec<input data-day-seconds="${index}" type="number" min="1" value="${data.config.daySeconds[index] || 5}" aria-label="Day ${index + 1} seconds"></label></div>`).join('');
  document.querySelectorAll('[data-day-steps]').forEach(input => input.addEventListener('change', () => { data.config.daySteps[Number(input.dataset.daySteps)] = Math.max(1, Number(input.value)); render(); }));
  document.querySelectorAll('[data-day-seconds]').forEach(input => input.addEventListener('change', () => { data.config.daySeconds[Number(input.dataset.daySeconds)] = Math.max(1, Number(input.value)); render(); }));
}
function renderAgents() {
  $('agent-list').innerHTML = agents.map((agent, index) => `<article class="agent-card ${index === selectedAgent ? 'selected' : ''}" data-agent="${index}"><div class="agent-card-top"><span class="agent-id"><b class="agent-icon">${index}</b> Agent ${index}</span><span>${agent.kind === 1 ? 'SUPPLY' : 'PATROL'}</span></div><div class="agent-meta"><span>POS ${agent.pos}</span><span>FUEL ${agent.fuel}/${data.config.fuelLimits}</span></div><div class="fuel-bar"><i style="width:${Math.max(0, agent.fuel / data.config.fuelLimits * 100)}%"></i></div></article>`).join('');
  document.querySelectorAll('.agent-card').forEach(card => card.addEventListener('click', () => { selectedAgent = Number(card.dataset.agent); render(); }));
}
function cellMarkup(x, y) {
  const pos = posId(x, y); const terrain = data.config.map.cells[y][x];
  const spot = data.config.spots.find(item => item.pos === pos);
  const traffic = currentState().traffics.find(item => item.pos === pos);
  const vehicles = agents.map((agent, i) => ({ ...agent, i, enemy: false })).concat(currentState().others.flatMap(team => team.agents.map((agent, i) => ({ ...agent, i, enemy: true, team: team.id })))).filter(agent => agent.pos === pos);
  return `<div class="hex ${['flat','road','mountain','pond'][terrain] || 'flat'}"><span class="hex-index">${pos}</span><span class="hex-label">${terrainName(terrain)}</span>${spot ? `<span class="entity spot" title="Brand ${spot.brand} · Stock ${spot.stocks}">${spot.brand}</span>` : ''}${traffic ? `<span class="entity traffic ${traffic.status === 1 ? 'busy' : traffic.status === 2 ? 'jammed' : ''}" title="Traffic status ${traffic.status}"></span>` : ''}${vehicles.map(vehicle => `<span class="entity vehicle ${vehicle.enemy ? 'enemy' : ''} ${!vehicle.enemy && vehicle.i === selectedAgent ? 'selected' : ''}" title="${vehicle.enemy ? `Team ${vehicle.team}` : `Agent ${vehicle.i}`} · Fuel ${vehicle.fuel}">${vehicle.enemy ? 'E' : vehicle.i}</span>`).join('')}</div>`;
}
function renderMap() { const rows = []; for (let y = 0; y < data.config.map.height; y++) rows.push(`<div class="hex-row ${y % 2 ? 'offset' : ''}">${Array.from({ length: data.config.map.width }, (_, x) => cellMarkup(x, y)).join('')}</div>`); $('map-wrap').innerHTML = `<div class="hex-grid">${rows.join('')}</div>`; }
function renderInspector() {
  const agent = agents[selectedAgent]; const point = agent ? posOf(agent.pos) : { x: 0, y: 0 }; const terrain = data.config.map.cells[point.y]?.[point.x];
  const traffic = currentState().traffics.find(item => item.pos === agent?.pos);
  $('inspector').innerHTML = agent ? `<div class="data-row"><span>Selected</span><strong>Agent ${selectedAgent}</strong></div><div class="data-row"><span>Type</span><strong>${agent.kind === 1 ? 'Supply' : 'Patrol'}</strong></div><div class="data-row"><span>Position</span><strong>${agent.pos} · (${point.x}, ${point.y})</strong></div><div class="data-row"><span>Fuel</span><strong>${agent.fuel} / ${data.config.fuelLimits}</strong></div><div class="data-row"><span>Terrain</span><strong>${terrainName(terrain)}</strong></div><div class="data-row"><span>Traffic</span><strong>${traffic ? ['Normal','Busy','Jammed'][traffic.status] || traffic.status : 'None'}</strong></div>` : '<p class="agent-meta">Select an agent to inspect it.</p>';
}
function renderEvents() { $('event-count').textContent = events.length; $('event-log').innerHTML = events.map(item => `<div class="event ${item.tone}"><time>${item.time}</time>${item.message}</div>`).join('') || '<p class="agent-meta">No events yet.</p>'; }
function move(direction) {
  const agent = agents[selectedAgent]; if (!agent || step >= data.config.daySteps[currentState().day]) return;
  const target = nextPosition(agent.pos, direction);
  if (!isWalkable(target)) { addEvent(`Agent ${selectedAgent} blocked at direction ${direction}`, 'warning'); return; }
  plannedActions = null; agent.pos = posId(target.x, target.y); agent.fuel = Math.max(0, agent.fuel - 1); step++; appendConsole(`Agent ${selectedAgent} moved to pos ${agent.pos}, fuel ${agent.fuel}`); addEvent(`Agent ${selectedAgent} moved direction ${direction} to ${agent.pos}`); render();
}
function waitStep() { if (step >= data.config.daySteps[currentState().day]) return; plannedActions = null; step++; appendConsole(`Agent ${selectedAgent} waited, fuel ${agents[selectedAgent].fuel}`); addEvent(`Agent ${selectedAgent} waited one step`); render(); }
async function requestSolverPlan() { const response = await fetch('/api/solve', { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify({ operation: 'solve', config: data.config, state: currentState() }) }); if (!response.ok) throw new Error(`Solver API HTTP ${response.status}`); const result = await response.json(); if (!Array.isArray(result.actions) || result.actions.length !== agents.length) throw new Error(result.error || 'Solver API returned an invalid action plan'); plannedActions = result.actions; addEvent(`Solver API returned a plan for day ${currentState().day}`); }
async function advanceStep() { if (advancing) return; advancing = true; try { const limit = data.config.daySteps[currentState().day] || 0; if (step >= limit) { playing = false; clearInterval(timer); if (!completedDays.has(dayIndex)) { completedDays.add(dayIndex); appendConsole(`Day ${currentState().day} completed`); } render(); return; } if (!plannedActions) await requestSolverPlan(); step++; agents.forEach((agent, index) => { const action = plannedActions[index]?.[step - 1]; if (Number.isInteger(action) && action >= 0 && action <= 5) { const target = nextPosition(agent.pos, action); if (isWalkable(target)) { agent.pos = posId(target.x, target.y); agent.fuel = Math.max(0, agent.fuel - 1); } } }); appendConsole(agents.map((agent, index) => `Agent ${index}: pos=${agent.pos} fuel=${agent.fuel}`).join(' | ')); addEvent(`Applied Solver action step ${step}`); render(); } catch (error) { playing = false; clearInterval(timer); addEvent(error.message, 'warning'); } finally { advancing = false; } }
function togglePlay() { playing = !playing; renderStatus(); if (playing) { timer = setInterval(advanceStep, 650); addEvent('Simulation started'); } else { clearInterval(timer); addEvent('Simulation paused'); } }
function runDay(index) { if (index > 0 && !completedDays.has(index - 1)) return; clearInterval(timer); playing = false; setDay(index, true); events = []; appendConsole(`Running day ${index + 1}`); render(); playing = true; timer = setInterval(advanceStep, 650); }

function createGeneratedData(profile) {
  const cells = Array.from({ length: profile.height }, () => Array.from({ length: profile.width }, () => Math.random() < 0.12 ? 3 : Math.floor(Math.random() * 3)));
  const walkable = []; cells.forEach((row, y) => row.forEach((terrain, x) => { if (terrain !== 3) walkable.push(posId(x, y)); }));
  randomize(walkable); const starts = walkable.slice(0, profile.agents); const spots = walkable.slice(profile.agents, profile.agents + profile.spots).map((pos, index) => ({ brand: index % 2, pos, stocks: 1 + index % 4 }));
  const config = { startsAt: Date.now(), daySeconds: [5, 5, 5, 5], daySteps: [20, 20, 20, 20], map: { height: profile.height, width: profile.width, cells }, spots, agents: starts, fuelLimits: profile.fuel, players: profile.players, busyThreshold: profile.busy, jammedThreshold: profile.jam };
  const baseAgents = starts.map(pos => ({ kind: 0, pos, fuel: profile.fuel }));
  const states = config.daySteps.map((_, day) => ({ endsAt: config.startsAt + day * 5 + 5, day, agents: structuredClone(baseAgents), others: [], traffics: walkable.slice(profile.agents + profile.spots, profile.agents + profile.spots + 8).map((pos, index) => ({ pos, status: index % 3 })) }));
  return { config, states };
}
function updateSettings(profile) { Object.entries({ width: profile.width, height: profile.height, fuel: profile.fuel, busy: profile.busy, jam: profile.jam, players: profile.players }).forEach(([key, value]) => { $(`setting-${key}`).value = value; }); $('difficulty-summary').textContent = `${profile.width} x ${profile.height} · ${profile.fuel} Fuel · ${profile.agents} xe · ${profile.spots} tiệm`; $('difficulty-note').textContent = `Ngẫu nhiên ${profile.width}x${profile.height}, ${profile.agents} xe Agents, ${profile.spots} tiệm Udon`; }
function generateMap() { const profile = difficultyProfiles[$('difficulty').value]; const settings = { width: Number($('setting-width').value), height: Number($('setting-height').value), fuel: Number($('setting-fuel').value), busy: Number($('setting-busy').value), jam: Number($('setting-jam').value), players: Number($('setting-players').value), agents: profile.agents, spots: profile.spots }; data = createGeneratedData(settings); completedDays = new Set(); events = []; consoleLines = []; selectedAgent = 0; setDay(0, true); addEvent('Generated a new local map'); }

document.querySelectorAll('[data-direction]').forEach(button => button.addEventListener('click', () => move(Number(button.dataset.direction))));
$('wait-button').addEventListener('click', waitStep); $('step-button').addEventListener('click', advanceStep); $('play-button').addEventListener('click', togglePlay); $('day-back').addEventListener('click', () => setDay(dayIndex - 1)); $('day-forward').addEventListener('click', () => setDay(dayIndex + 1));
$('difficulty').addEventListener('change', () => updateSettings(difficultyProfiles[$('difficulty').value])); $('generate-button').addEventListener('click', generateMap); $('clear-console').addEventListener('click', () => { consoleLines = []; renderConsole(); });
$('reset-button').addEventListener('click', () => { playing = false; clearInterval(timer); data = structuredClone(seed); completedDays = new Set(); events = []; consoleLines = []; selectedAgent = 0; setDay(0, true); });
$('json-file').addEventListener('change', async (event) => { const file = event.target.files[0]; if (!file) return; try { const text = await file.text(); const values = text.trim().split(/\n(?=\s*\{)/).map(line => JSON.parse(line)); data = { config: values[0], states: values.slice(1) }; setDay(0); addEvent(`Loaded ${file.name}`); } catch (error) { addEvent('Could not parse JSON file', 'warning'); } });
updateSettings(difficultyProfiles.easy); data = createGeneratedData({ ...difficultyProfiles.easy }); setDay(0, true);
