#include "Scraper.h"

bool iequals(std::string_view a, std::string_view b)
{
    if (a.size() != b.size()) return false;

    return std::equal(a.begin(), a.end(), b.begin(),
        [](char c1, char c2) {
            return std::tolower(static_cast<unsigned char>(c1)) ==
                std::tolower(static_cast<unsigned char>(c2));
        });
}

// Callback-функция для получения данных от curl
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* userp)
{
    size_t realsize = size * nmemb;
    userp->append(static_cast<char*>(contents), realsize);
    return realsize;
}

// Функция загрузки HTML-страницы
int DownloadHTML(const string& url, string* html)
{
    if (!html) {
        cerr << "Error: pointer html == nullptr\n";
        return 1;
    }

    // Очищаем строку на всякий случай
    html->clear();

    CURL* curl = curl_easy_init();
    if (!curl) {
        cerr << "Failed to initialize curl\n";
        return 1;
    }

    // Основные настройки
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, html);

    // Полезные и часто рекомендуемые настройки
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);       // следовать редиректам
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 10L);           // максимум редиректов
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (compatible; SimpleBot/1.0)");
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip, deflate"); // поддержка сжатия

    // Таймауты (рекомендуется)
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 15L);     // 15 секунд на соединение
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);             // общий таймаут 30 секунд

    // Выполнение запроса
    CURLcode res = curl_easy_perform(curl);

    // Проверка результата
    if (res != CURLE_OK) {
        cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << '\n';
        curl_easy_cleanup(curl);
        return 1;
    }

    // Успешно
    cout << "Loaded success! " << html->length() << " byte\n";

    curl_easy_cleanup(curl);
    return 0;
}


void find_buttons_and_images(const GumboNode* node)
{
    if (node->type != GUMBO_NODE_ELEMENT)
        return;

    const GumboElement& elem = node->v.element;
    GumboTag tag = elem.tag;

    // -----------------------
    //        Кнопки
    // -----------------------
    if (tag == GUMBO_TAG_BUTTON)
    {
        std::cout << "Found <button>";

        // Попробуем достать текст кнопки (самый простой случай)
        const GumboVector* children = &elem.children;
        if (children->length > 0)
        {
            const GumboNode* child = static_cast<GumboNode*>(children->data[0]);
            if (child->type == GUMBO_NODE_TEXT)
            {
                std::cout << " → text: \"" << child->v.text.text << "\"";
            }
        }
        std::cout << "\n";
    }
    else if (tag == GUMBO_TAG_INPUT)
    {
        const GumboAttribute* type_attr = gumbo_get_attribute(&elem.attributes, "type");
        if (type_attr &&
            (iequals(type_attr->value, "button") == 0 ||
      
                iequals(type_attr->value, "submit") == 0 ||
                iequals(type_attr->value, "reset") == 0))
        {
            const GumboAttribute* value_attr = gumbo_get_attribute(&elem.attributes, "value");
            std::cout << "Found <input type=\"" << (type_attr->value ? type_attr->value : "?")
                << "\">";

            if (value_attr)
                std::cout << " → value: \"" << value_attr->value << "\"";

            std::cout << "\n";
        }
    }

    // -----------------------
    //        Картинки
    // -----------------------
    if (tag == GUMBO_TAG_IMG)
    {
        const GumboAttribute* src = gumbo_get_attribute(&elem.attributes, "src");
        const GumboAttribute* alt = gumbo_get_attribute(&elem.attributes, "alt");

        std::cout << "Found <img>";

        if (src)
            std::cout << " src=\"" << src->value << "\"";
        else
            std::cout << " src=(отсутствует)";

        if (alt)
            std::cout << " alt=\"" << alt->value << "\"";

        std::cout << "\n";
    }

    // Рекурсивный обход детей
    const GumboVector* children = &elem.children;
    for (unsigned int i = 0; i < children->length; ++i)
    {
        find_buttons_and_images(static_cast<const GumboNode*>(children->data[i]));
    }
}

// Рекурсивный поиск всех ссылок <a href="...">
void find_all_links(const GumboNode* node, vector<string>& links)
{
    if (node->type != GUMBO_NODE_ELEMENT)
        return;

    if (node->v.element.tag == GUMBO_TAG_A)
    {
        const GumboAttribute* href = gumbo_get_attribute(&node->v.element.attributes, "href");
        if (href)
            links.push_back(href->value);
    }

    const GumboVector* children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i)
    {
        find_all_links(static_cast<GumboNode*>(children->data[i]), links);
    }
}

std::string readAll(const std::string& fileName)
{
    std::ifstream ifs;
    ifs.open(fileName);
    ifs.seekg(0, std::ios::end);
    size_t length = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
    std::string buff(length, 0);
    ifs.read(&buff[0], length);
    ifs.close();

    return buff;
}

void PrettyJSON(rapidjson::Document &Doc, string& FileName)
{
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

    // Настраиваем красоту (по желанию)
    writer.SetIndent(' ', 2);          // 2 пробела (по умолчанию 4)
    writer.SetFormatOptions(
        rapidjson::kFormatSingleLineArray   // ← массивы в одну строку, если короткие
        // или убрать эту строку — тогда даже короткие массивы будут с переносами
    );

    Doc.Accept(writer);  // ← здесь магия
    if (!FileName.empty())
    {
        ofstream JSONFile(FileName);
        string JSON = buffer.GetString();
        JSONFile << JSON;
        JSONFile.close();
    }
}



int main()
{
    cout << "Parsing link of pictures!" << endl;
    
    string APIKey = "";
    string query = "dog";
    int page = 1;
    string response;
    vector<string> ListUrls;
    cout << "Target: " << query << endl;
    while(page < 80)
    {
        std::string url = "https://api.pexels.com/v1/search"
            "?query=" + query +
            "&page=" + to_string(page) +
            "&per_page=80";
        if (DownloadHTML(url, &response) != 0)
        {
            cerr << "Failed to download HTML!";
            return -1;
        }
        size_t start = 0;
        if (response.size() >= 3 &&
            static_cast<unsigned char>(response[0]) == 0xEF &&
            static_cast<unsigned char>(response[1]) == 0xBB &&
            static_cast<unsigned char>(response[2]) == 0xBF)
        {
            start = 3;
            std::cout << "BOM found and deleted!\n";
        }
        if (response.empty())
        {
            cerr << "ERROR: Data is empty!" << endl;
            return -1;
        }
        rapidjson::Document Doc;

        Doc.Parse<rapidjson::kParseStopWhenDoneFlag>(
            response.data() + start,
            response.size() - start
        );
        if (Doc.HasParseError()) {
            // Можно вывести подробности ошибки (очень полезно при отладке)
            std::cerr << "JSON is not valid. Error: "
                << rapidjson::GetParseError_En(Doc.GetParseError())
                << " (position " << Doc.GetErrorOffset() << ")\n";

            // Примеры типичных сообщений:
            //   "The document is empty."
            //   "Missing a name for object member."
            //   "Incorrect syntax"
            //   "The value is invalid."

            return -2;
        }
        if (!Doc.HasMember("photos") || Doc["photos"].Empty())
        {
            break;
        }
        
        const auto& photos = Doc["photos"].GetArray();

        for (const auto& photo : photos)
        {
            // photo — это объект { "id": ..., "src": { ... }, ... }

            if (!photo.IsObject()) continue;

            if (photo.HasMember("src") && photo["src"].IsObject())  // ← src — объект, а НЕ массив
            {
                const auto& src = photo["src"];

                if (src.HasMember("original") && src["original"].IsString())
                {
                    ListUrls.emplace_back(src["original"].GetString());
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1200));
        page++;
    }
    // Парсим с помощью Gumbo
    vector<string> links;

    // Выводим результат
    ofstream File("Pictures_Links.txt");
    for (auto Link : ListUrls)
    {
        File << Link << endl;
        cout << Link << endl;
    }
    cout << "Total Links for pictures: " << ListUrls.size() << endl;
    cout << "Done!" << endl;
    system("pause");
    File.close();
    
    
    return 0;
}

