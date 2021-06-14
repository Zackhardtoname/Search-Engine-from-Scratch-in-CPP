from bs4 import BeautifulSoup
import requests

with open("../server/urls.txt") as f:
    content = f.readlines()

urls = [url for url in content]

# Make a GET request to fetch the raw HTML content
# html_content = requests.get(url).text

import grequests

lst = ['https://www.google.com', 'https://www.google.cz']
class Test:
    def __init__(self):
        self.urls = lst

    def exception(self, request, exception):
        print ("Problem: {}: {}".format(request.url, exception))

    def f_async(self):
        return grequests.map((grequests.get(u) for u in self.urls), exception_handler=self.exception, size=5)


    def collate_responses(self, results):
        return [x.text for x in results]

test = Test()
#here we collect the results returned by the async function
results = test.f_async()
response_text = test.collate_responses(results)



# Parse the html content
# soup = BeautifulSoup(html_content, "lxml")
