from pathlib import Path
from request_tests import *

TEST_DIR = "./build/test/"
APIS = ["employee", "position", "newsletter", "stat" ]

def writeTxt(fName, value):
    with open(fName, 'a') as f:
        f.write(str(value))

def clearFile(fName):
    with open(fName, 'w') as f:
        f.write("")

def testApiRequests(api):
    fName = f"{TEST_DIR}{api}.txt"
    clearFile(fName)

    writeTxt(fName,f"======== GET request of {api} - {REQUEST}{api} ========\n\n")
    response = get_paginated_request(api)
    writeTxt(fName, f"{response.text}\n")

    writeTxt(fName,f"\n======== GET by id request of {api} - {REQUEST}{api} ========\n\n")
    response = get_by_id_request(api)
    writeTxt(fName, f"{response.text}\n")

    writeTxt(fName,f"\n======== DELETE by id request of {api} - {REQUEST}{api} ========\n\n")
    response = delete_request(api)
    writeTxt(fName, f"{response.text}\n")

    writeTxt(fName,f"\n======== POST request of {api} - {REQUEST}{api} ========\n\n")
    response = post_request(api)
    writeTxt(fName, f"{response.text}\n")

    writeTxt(fName,f"\n======== PUT request of {api} - {REQUEST}{api} ========\n\n")
    response = put_request(api)
    writeTxt(fName, f"{response.text}\n")

    writeTxt(fName,f"\n======== DB after changes ========\n\n")
    response = get_paginated_request(api)
    writeTxt(fName, f"{response.text}\n")


def main():
    test_dir_path = Path(TEST_DIR)
    try:
        test_dir_path.mkdir()
    except FileExistsError:
        print(f"Test directory already exists.")
    except PermissionError:
        print(f"Permission denied: Unable to create '{test_dir_path}'.")

    for api in APIS:
        testApiRequests(api)

        print(f"api -\t{REQUEST}{api}  \t - tested succesfully")
    return


if __name__ == "__main__":
    main()