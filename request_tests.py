import requests
import json

PAGE=1
PAGESIZE=10
PORT=8080
REQUEST = f"http://localhost:{PORT}/api/"
EXAMPLE_BODIES = {
    "employee": {
        "login": "johndoe",
        "name": "John",
        "surname": "Doe",
        "employment_date": "2025-11-04",
        "position_id": 1,
        "role_id": 1,
        "password": "1111"
    },
    "position": {
        "title": "New Senior Programmer title",
        "Salary": 2500
    },
    "newsletter": {
        "title": "New newsletter",
        "body": "This is new newsletter body",
        "date": "2025-01-01"
    },
    "stat":{
        "date": "2025-01-15",
        "time": "14:32:10",
        "clock_amount": 5,
        "employee_id": 0  # should find first existing employee ID
    }
}



def get_field(api, index):
    keys = list(EXAMPLE_BODIES[api].keys())
    values = list(EXAMPLE_BODIES[api].values())
    return keys[index], values[index]

def get_paginated_request(api):
    response = requests.get(f"{REQUEST}{api}s?page={PAGE}&page_size={PAGESIZE}")
    return response

def get_by_id_request(api):
    response = get_paginated_request(api)
    data = json.loads(response.text)

    existing_id = data["items"][0]["id"]

    response = requests.get(f"{REQUEST}{api}/{existing_id}")
    return response


def delete_request(api):
    response = get_paginated_request(api)
    data = json.loads(response.text)
    existing_id = data["items"][0]["id"]

   
    response = requests.delete(f"{REQUEST}{api}/{existing_id}")
    
    return response

def post_request(api):
    body = EXAMPLE_BODIES[api]

    if api == "stat":
        response = get_paginated_request("employee")
        data = json.loads(response.text)
        body["employee_id"] = data["items"][0]["id"]
    
    response = requests.post(url=f"{REQUEST}{api}",
                        headers={"Content-Type": "application/json"},
                        data=json.dumps(body))

    return response

def put_request(api):
    updated_field = 2 if api == "stat" else 0
    
    key, value = get_field(api, updated_field)
    
    response = requests.get(f"{REQUEST}{api}s")

    data = json.loads(response.text)

    if isinstance(value, int):
        data["items"][0][key] = value + 1
    else:
        data["items"][0][key] = f"updated {value}"
    
    response = requests.put(url=f"{REQUEST}{api}",
                                headers={"Content-Type": "application/json"},
                                data=json.dumps(data["items"][0]))
    
    return response
