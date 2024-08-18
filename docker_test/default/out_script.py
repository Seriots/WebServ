import requests


server_list = {
    "http://webserv.42.fr:8080": [200, "/root/index_all.html"],
    "http://192.168.12.1:8080": [200, "/root/index_all.html"],
    "http://out.webserv.42.fr:8080": [200, "/root/index_coucou.html"],
    "http://webserv.42.fr:8080/test/index.html": [200, "/root/index_test.html"],
    "http://webserv.42.fr:8080/a/b/index.html": [200, "/root/index_test.html"],
    "http://webserv.42.fr:8080/c/index.html": [405, ""]
    }


class Test:
    def __init__(self, url, status, filename):
        self.url = url
        self.status = status
        self.filename = filename

    def basic_get_test(self):
        req = requests.get(self.url)
        print(f"{self.url}: ", end="")
        print(f"expect {self.status} got {req.status_code}", end="")
        if self.filename:
            with open(self.filename, 'r') as expected:
                if expected.read() == req.text:
                    print(": content ok", end="")
                else:
                    print(": content ko", end="")
        print("")

    def cookie_test(self):
        req = requests.get(self.url)
        print(f"{self.url}: ")
        print(f"no cookie: -> {req.cookies['WebServCookie']} | {req.headers['Pragma']}")
        req2 = requests.get(self.url, cookies={"WebServCookie": "testtesttest"})
        print(f"with cookie: -> {req2.headers['Pragma']}")


def server_selection(servers):
    print("basic get".center(15, '-'))
    for page in servers:
        t = Test(page, *servers[page])
        t.basic_get_test()

def server_cookies(servers):
    print("servers cookies".center(15, '-'))
    for page in servers:
        t = Test(page, *servers[page])
        t.cookie_test()


def main(servers):
    print("Start testing".center(30, "="))
    server_selection(servers)
    server_cookies(servers)
    print("End testing".center(30, "="))


if __name__ == "__main__":
    main(server_list)
