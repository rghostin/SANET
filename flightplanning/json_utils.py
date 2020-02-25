import json

def json_positionsFromList(vec_of_positions):
    """ [(pos)] to {"0":(pos); "1":...}"""
    return json.dumps({i:vec_of_positions[i] for i in range(len(vec_of_positions))})
