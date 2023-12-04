#!/usr/bin/env python3

from typing import List, Tuple
from dataclasses import dataclass

from state import State
from actions import ActionBase, get_available_actions


@dataclass
class SearchState:
    state: State
    action: ActionBase = None
    parent: 'SearchState' = None

    def __hash__(self):
        return hash(self.state)

    # FIXME remove cost and score, compute this in heuristic function
    cost: float = 0
    # The score of the state, lower is better
    score: float = 0
    num_actions: int = 0


@dataclass
class SearchStats:
    num_states_explored: int = 0
    max_frontier_size: int = 0

    def update(self, frontier_size: int):
        self.max_frontier_size = max(self.max_frontier_size, frontier_size)
        self.num_states_explored += 1


def get_idx_of_state_with_lowest_score(
        search_states: List[SearchState]) -> int:
    min_idx = 0
    min_score = None
    for idx, search_state in enumerate(search_states):
        if min_score is None or min_score > search_state.score:
            min_score = search_state.score
            min_idx = idx
    return min_idx


def expand_state(search_state: SearchState,
                 get_state_score) -> List[SearchState]:
    actions: List[ActionBase] = get_available_actions(search_state.state)
    new_search_states = []
    for action in actions:
        new_state = action.get_state_from(search_state.state)
        #print("exploring:", new_state)
        cost = action.compute_cost(search_state.state)

        new_search_state = SearchState(new_state, action, search_state)
        new_search_state.cost = search_state.cost + cost
        new_search_state.score = get_state_score(new_search_state)
        new_search_state.score += new_search_state.cost
        new_search_state.num_actions = search_state.num_actions + 1
        #print(new_search_state.num_actions, new_search_state.score, action)

        new_search_states.append(new_search_state)
    return new_search_states


def a_star_search(initial_state, get_state_score,
                  is_goal_state) -> Tuple[SearchState, SearchStats]:
    frontier = [initial_state]
    stats = SearchStats()

    while len(frontier):
        stats.update(len(frontier))

        idx = get_idx_of_state_with_lowest_score(frontier)
        next_state = frontier.pop(idx)
        if is_goal_state(next_state):
            return next_state, stats
        for new_state in expand_state(next_state, get_state_score):
            frontier.append(new_state)
