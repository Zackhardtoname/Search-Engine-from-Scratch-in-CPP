//
// Created by youh on 3/14/21.
//

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: ./crawler <URL>" << std::endl;
        return 1;
    }

    // TODO: convert this into the main controller thread that will be
    //  calling a url on this crawler.
    auto httpResp = FetchUrl(argv[1]);
    // auto httpResp = GetHtml("http://anglesharp.azurewebsites.net/Chunked"); // test chunked
    std::cout << "code: " << httpResp.StatusCode << '\n'
              << "location: " << httpResp.Location << '\n'
              << "lang: " << httpResp.Language << '\n'
              << "---- body ----" << '\n'
              << httpResp.Body;
    return 0;
}

