#!/usr/bin/env python3
"""
A* State Space Exploration Prototype for RPG
"""

import sys
import signal
import time

from config import load_config

from state import State

from a_star import SearchState, a_star_search


def dump_search(initial_state: State, get_state_score, is_goal_state):
    print("Initial State:")
    print(initial_state)

    start_time = time.time()
    final_state, stats = a_star_search(SearchState(initial_state),
                                       get_state_score, is_goal_state)
    end_time = time.time()
    delta_time_ms = (end_time - start_time) * 1000

    print("=" * 80)
    print("Search done:")
    print(f"Time: {delta_time_ms:.2f}ms")
    print(f"Number of states explored: {stats.num_states_explored}")
    print(f"Max Frontier Size: {stats.max_frontier_size}")
    print("=" * 80)

    if final_state is None:
        print("No final state found")
        return

    print("Actions:")
    action_infos = []
    last_state = final_state
    while last_state is not None and last_state.action is not None:
        action_infos.append(
            (last_state.action, last_state.cost, last_state.score))
        last_state = last_state.parent
    for idx, action_info in enumerate(reversed(action_infos)):
        action, cost, score = action_info
        print(f"{idx+1}.", action, "Cost:", cost, "Score:", score)

    print("Final State:")
    print(final_state.state)
    print("#" * 80)


def _get_stats_score(state: State,
                     w_health: int = -5,
                     w_hunger: int = -5,
                     w_thirst: int = -5,
                     w_fatigue: int = -5) -> int:
    score = 0
    score += w_health * (1000 - state.health)
    score += w_hunger * (1000 - state.hunger)
    score += w_thirst * (1000 - state.thirst)
    score += w_fatigue * (1000 - state.fatigue)
    return score

def _get_inventory_penalty(state: State) -> int:
    return state.inventory.num_items * 5


def is_not_thirsty(search_state: SearchState) -> bool:
    return search_state.state.thirst >= 1000


def get_thirst_score(search_state: SearchState) -> int:
    score = 0
    score += _get_stats_score(search_state.state, w_thirst=1)
    score += _get_inventory_penalty(search_state.state)
    return score


def is_not_hungry(search_state: SearchState) -> bool:
    return search_state.state.hunger >= 1000


def get_hunger_score(search_state: SearchState) -> int:
    score = 0
    score += _get_stats_score(search_state.state, w_hunger=1)
    score += _get_inventory_penalty(search_state.state)
    return score


def is_not_sleepy(search_state: SearchState) -> bool:
    return search_state.state.fatigue >= 1000


def get_fatigue_score(search_state: SearchState) -> int:
    score = 0
    score += _get_stats_score(search_state.state, w_fatigue=1)
    score += _get_inventory_penalty(search_state.state)
    return score


def is_healed(search_state: SearchState) -> bool:
    return search_state.state.health >= 1000


def get_health_score(search_state: SearchState) -> int:
    score = 0
    score += _get_stats_score(search_state.state, w_health=1)
    score += _get_inventory_penalty(search_state.state)
    return score


def run_scenario(config_path: str):
    cfg = load_config(config_path)
    ghm = {
        "thirst": (get_thirst_score, is_not_thirsty),
        "hunger": (get_hunger_score, is_not_hungry),
        "health": (get_health_score, is_healed),
        "fatigue": (get_fatigue_score, is_not_sleepy),
    }
    get_state_score, is_goal_state = ghm[cfg.goal_heuristic.lower()]

    print("=" * 80)
    print(f"Description: {cfg.description:s}")
    print(f"Goal Heuristic: {cfg.goal_heuristic:s}")
    print("=" * 80)
    dump_search(cfg.initial_state, get_state_score, is_goal_state)


def _runtime_timeout(_signum, _frame):
    print("=" * 80)
    print("ERROR TIMEOUT")
    print("=" * 80)
    sys.exit(1)


if __name__ == "__main__":
    assert len(sys.argv) >= 2, "not enough arguments"
    scenario_cfg = sys.argv[1]
    timeout = int(sys.argv[2]) if len(sys.argv) >= 3 else None

    if timeout is not None:
        signal.signal(signal.SIGALRM, _runtime_timeout)
        signal.alarm(timeout)
    run_scenario(scenario_cfg)
