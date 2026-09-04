# Game System API Specification

## 1. Overview
This document defines the REST API for a turn-based programming game system (Procon style). 
The system consists of two main parts:
- **Game API**: Used by Player Bots to read match status and submit moves.
- **Admin API**: Used by the Game Server/Admin to manage teams, matches, and control the match lifecycle.

**Target Audience**: This specification is designed for AI coding agents and developers to implement the backend server or the player bot clients.

---

## 2. Authentication
- **Game API** requires an API token.
- **Header format**: `x-api-token: <your_team_api_token>`
- **Admin API** does not use `x-api-token` (assume separate Admin authentication/authorization).

---

## 3. Game API (Player Bots)
All endpoints here require the `x-api-token` header.

### 3.1. List Matches
- **Method**: `GET`
- **Endpoint**: `/api/game/matches`
- **Description**: List all matches assigned to the authenticated team.

### 3.2. Get Map Configuration
- **Method**: `GET`
- **Endpoint**: `/api/game/matches/:id/config`
- **Description**: Retrieve the map configuration and rules before a match begins.

### 3.3. Submit Agent Selection
- **Method**: `POST`
- **Endpoint**: `/api/game/matches/:id/agents`
- **Description**: Submit the initial selection of agent types for the match.

### 3.4. Get Match Status
- **Method**: `GET`
- **Endpoint**: `/api/game/matches/:id/status`
- **Description**: Get the current "daily" (turn) status of the match in Procon format.

### 3.5. Submit Action Plan
- **Method**: `POST`
- **Endpoint**: `/api/game/matches/:id/answer`
- **Description**: Submit the action plan (moves/commands) for the current turn.

---

## 4. Admin API (Management)
These endpoints are used to control the game state and entities.

### 4.1. Team Management
- `GET /api/admin/teams`: List all teams.
- `POST /api/admin/teams`: Create a new team.
- `DELETE /api/admin/teams/:id`: Delete a team.

### 4.2. Match Management
- `GET /api/admin/matches`: List all matches.
- `POST /api/admin/matches`: Create a match (includes initial map config payload).
- `POST /api/admin/matches/:id/teams`: Add a team to a match.
- `DELETE /api/admin/matches/:id/teams`: Remove a team from a match.

### 4.3. Match Lifecycle Control
- **Method**: `PUT`
- **Endpoint**: `/api/admin/matches/:id`
- **Description**: Controls the state machine of the match.
- **Expected Payload**: `{ "state": "<target_state>" }`
  - Valid states: 
    - `start_agent_select`: Opens the window for teams to POST `/agents`.
    - `start`: Begins the first turn.
    - `next_day`: Advances the match to the next turn (day).
    - `end`: Terminates the match.

---

## 5. Implementation Workflow (For AI Agent Context)

If you are instructed to implement the **Bot Client**, follow this loop:
1. `GET /api/game/matches` to find active matches.
2. `GET /api/game/matches/:id/config` to prepare logic.
3. `POST /api/game/matches/:id/agents` when agent selection starts.
4. **Main Game Loop:**
   - Poll `GET /api/game/matches/:id/status` waiting for a new day/turn.
   - Calculate next moves based on status.
   - `POST /api/game/matches/:id/answer` before the turn time expires.

If you are instructed to implement the **Backend Server**, ensure:
1. Matches have a strict State Machine (`CREATED` -> `AGENT_SELECT` -> `PLAYING` -> `FINISHED`).
2. Only allow `/api/game/matches/:id/answer` if the match state is `PLAYING`.
3. Validating the `x-api-token` middleware is applied to all `/api/game/*` routes.
