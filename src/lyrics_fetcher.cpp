#include "lyrics_fetcher.h"

size_t writeToString(void *ptr, size_t size, size_t count, void *stream);
void replaceAll(string& str, const string& from, const string& to);

void fetch_lyrics(Fl_Text_Buffer *lyrics_text_buffer, string artist, string title)
{
CURL *curl;
    string data;

    //Yeah, lyricswikia, it pretty reliable.
    //Maybe we can upgrade this code to support
    //several sites, if one fail, try the next.
    //I was wondering to user Google Cache so even
    //at work behind a firewall(Google services are the only exceptions)
    //it would be possible to fetch the lyrics. But suddendly
    //Google has started to use a Hash on the url :(
    string url = "http://lyrics.wikia.com/";

    //As of May 2012, these regex are valid.
    //If they change the site layout, this fetcher
    //might not work properly or not work at all!
    string conditionNotFound = "This page needs content.";
    string conditionNotFound2 = "PUT LYRICS HERE";
    string conditionStart = "lyrics>";
    string conditionEnd = "&lt;/lyrics>";

    replaceAll(artist, " ", "_");
    replaceAll(title, " ", "_");

    url = url.append(artist);
    url = url.append(":");
    url = url.append(title);
    url = url.append("?action=edit");

	//To remove, using while debugging
	cout << url << endl;

    curl = curl_easy_init();
    if(curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToString);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
        CURLcode res = curl_easy_perform(curl);

        //Check if there was any problem
        if(res != CURLE_OK)
        {
            curl_easy_cleanup(curl);
            lyrics_text_buffer->text("Connection failure!");
            return;
        }

        //Check if the Lyrics was found
        //if not, the string conditionNotFound will be at the page
        int a = data.find(conditionNotFound);
        if(a != string::npos)
        {
            curl_easy_cleanup(curl);
            lyrics_text_buffer->text("Not found!");
            return;
        }

        a = data.find(conditionNotFound2);
        if(a != string::npos)
        {
            curl_easy_cleanup(curl);
            lyrics_text_buffer->text("Not found!");
            return;
        }

        int b = data.find(conditionStart);
        if(b == -1)
        {
            curl_easy_cleanup(curl);
            lyrics_text_buffer->text("Error while fetching.");
            return;
        }
        data.erase(0, b+conditionStart.size());

        int c = data.find(conditionEnd);
        data.erase(c);

        curl_easy_cleanup(curl);

        //It's necessary as a workaround for FLTK Fl_Help_View widget.
        //As of version 1.3.0, multiple <br> tags are handled as only
        //one, so we use to use the <p> tag.
        replaceAll(data, "<br /><br />", "</p><p>");
    }

    replaceAll(artist, "_", " ");
    replaceAll(title, "_", " ");

    string result = artist+"\n"+title+"\n"+data;
    replaceAll(result, "\n\n\n", "\n\n");
    while(result.at(result.size()-1) == '\n')
        result = result.substr(0, result.size()-1);
    lyrics_text_buffer->text(result.c_str());
}

size_t writeToString(void *ptr, size_t size, size_t count, void *stream)
{
    ((string*)stream)->append((char*)ptr, 0, size* count);
    return size* count;
}

void replaceAll(string& str, const string& from, const string& to)
{
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}
