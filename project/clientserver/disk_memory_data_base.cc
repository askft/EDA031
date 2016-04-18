#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <fstream>
#include <algorithm>
#include <regex>

#include "database.h"
#include "article.h"
#include "newsgroup.h"
#include "disk_memory_data_base.h"
#include "protocol.h"

using namespace std;



DiskMemoryDataBase::DiskMemoryDataBase(const string& root_path)
    : root_directory_path{root_path}
{
    DIR *root_dp;
    struct dirent *root_dirp;
    string newsgroup_path, article_path;
    string article_author, article_text = "", article_title;
    int article_id;
    ifstream article_stream;
    
    //Read/create root directory
    if ((root_dp = opendir(root_directory_path.c_str())) == NULL) {
        int status;
        status = mkdir(root_directory_path.c_str(), S_IRWXU);
        if (status != 0) {
            cout << "Error(" << errno << ") creating " << root_directory_path << endl;
        }
        else {
            root_dp = opendir(root_directory_path.c_str());
        }
    }
    
    //Create init_file if it doesn't exist, read it otherwise
    ifstream init_stream;
    init_stream.open(root_directory_path + "/init_file");
    if (!init_stream.good()) {
        newsgroup_id = 0;
        article_id = 0;
        write_to_init(article_id, newsgroup_id);
    } else {
        read_init();
    }
    
    while ((root_dirp = readdir(root_dp)) != NULL) {
        //Create newsgroup object from directory
        if (!strcmp(root_dirp->d_name, ".") ||
            !strcmp(root_dirp->d_name, "..") ||
            !strcmp(root_dirp->d_name, ".DS_Store") ||
            !strcmp(root_dirp->d_name, "init_file")) {
            continue;
        }
        
        //Save newsgroup as object
        int current_newsgroup_id = get_id(root_dirp->d_name);
        string current_newsgroup_name = get_name(root_dirp->d_name);
        Newsgroup newsgroup(current_newsgroup_id, current_newsgroup_name);
        
        //Open newsgroup directory
        DIR *newsgroup_dp;
        struct dirent *newsgroup_dirp;
        newsgroup_path = root_directory_path + "/" + root_dirp->d_name;
        cout << newsgroup_path << endl;
        newsgroup_dp = opendir(newsgroup_path.c_str());
        
        //read files in newsgroup directory
        while ((newsgroup_dirp = readdir(newsgroup_dp)) != NULL) {
            if (!strcmp(newsgroup_dirp->d_name, ".") ||
                !strcmp(newsgroup_dirp->d_name, "..") ||
                !strcmp(root_dirp->d_name, ".DS_Store")) {
                continue;
            }
            article_title = get_name(newsgroup_dirp->d_name);
            article_id = get_id(newsgroup_dirp->d_name);
            article_path = newsgroup_path + "/" + newsgroup_dirp->d_name;
            article_stream.open(article_path.c_str());
            
            string line, stringed_article_id;
            getline(article_stream, article_author); //First line = author
            while (getline(article_stream, line)) {
                article_text += line + "\n";
            }
            Article article(article_title, article_author, article_text);
            article_stream.close();
            newsgroup.addArticle(article);
        }
        closedir(newsgroup_dp);
        newsgroups.push_back(newsgroup);
    }
    closedir(root_dp);
}

vector<pair<string, unsigned int>>
DiskMemoryDataBase::listNewsgroups()
{
    vector<pair<string, unsigned int>> vec;
    for (Newsgroup newsgroup : newsgroups) {
        vec.push_back(make_pair(newsgroup.name(), newsgroup.id()));
    }
    return vec;
}

int
DiskMemoryDataBase::addNewsgroup(
             string newsgroup_title)
{
    auto itr = find_if(newsgroups.begin(), newsgroups.end(),
             [newsgroup_title](Newsgroup ng) {
                 return get_name(ng.name()) == newsgroup_title;
             });
    
    if (itr == newsgroups.end()) {
        ostringstream convert;
        convert << newsgroup_id;
        string newsgroup_path = root_directory_path + "/" + convert.str() + "_" + newsgroup_title;
        int status = mkdir(newsgroup_path.c_str(), S_IRWXU);
        if (status != 0) {
            cout << "Error(" << errno << ") creating " << newsgroup_path << endl;
            //Borde returnera ett error här. Vet dock inte vad det skulle vara...
        }
        Newsgroup newsgroup(newsgroup_id, newsgroup_title);
        newsgroups.push_back(newsgroup);
        ++newsgroup_id;
        return Protocol::ANS_ACK;
    } else {
        return Protocol::ERR_NG_ALREADY_EXISTS;
    }
}

int
DiskMemoryDataBase::deleteNewsgroup(
                string newsgroup_title)
{
    for (auto itr = newsgroups.begin(); itr != newsgroups.end(); ++itr) {
        if (itr->name() == newsgroup_title) {
            string newsgroup_path = root_directory_path + "/" + itr->name();
            remove(newsgroup_path.c_str());
            newsgroups.erase(itr);
            return Protocol::ANS_ACK;
        }
    }
    return Protocol::ERR_NG_DOES_NOT_EXIST;
}

vector<Article>
DiskMemoryDataBase::getArticles(
            string newsgroup_title)
{
    vector<Article> vec;
    for (Newsgroup newsgroup : newsgroups) {
        if (newsgroup.name() == newsgroup_title) {
            vec = newsgroup.listNewsgroup();
            return vec;
        }
    }
    return vec;
}

void
DiskMemoryDataBase::addArticle(
           string newsgroup_title,
           string article_name,
           string author,
           string text)
{
    
    Article article(article_name, author, text);
    auto itr = find_if(newsgroups.begin(), newsgroups.end(),
                       [newsgroup_title](Newsgroup ng) {
                           return ng.name() == newsgroup_title;
                       });
    
    if (itr != newsgroups.end()) {
        itr->addArticle(article);
        
    } else {
        addNewsgroup(newsgroup_title);
        auto& newsgroup = newsgroups.back();
        newsgroup.addArticle(article);
    }
    
    string newsgroup_path = root_directory_path + "/" + newsgroup_title;
    ofstream article_file;
    article_file.open(newsgroup_path + "/" + article_name);
    article_file << article_id << "\n";
    article_file << author + "\n";
    article_file << text;
    ++article_id;
    article_file.close();
    
    
}

int
DiskMemoryDataBase::deleteArticle(
              string newsgroup_title,
              string article_name)
{
    string article_path = root_directory_path + "/" + newsgroup_title + "/" + article_name;
    auto itr = find_if(newsgroups.begin(), newsgroups.end(),
                       [newsgroup_title](Newsgroup ng) {
                           return newsgroup_title == ng.name();
                       });
    if (itr != newsgroups.end()) {
        if (itr->deleteArticle(article_name)) {
            remove(article_path.c_str());
            return Protocol::ANS_ACK;
        } else {
            return Protocol::ERR_ART_DOES_NOT_EXIST;
        }
    } else {
        return Protocol::ERR_NG_DOES_NOT_EXIST;
    }
}

string
DiskMemoryDataBase::getArticle(
           string newsgroup_title,
           string article_name)
{
    auto itr = find_if(newsgroups.begin(), newsgroups.end(),
                       [newsgroup_title](Newsgroup ng) {
                           return newsgroup_title.compare(ng.name()) == 0;
                       });
   
    if (itr != newsgroups.end()) {
        return itr->getArticle(article_name);
    } else {
        return "Error: could not find newsgroup";
    }
}

void
DiskMemoryDataBase::write_to_init(
            int article_id,
            int newsgroup_dp)
{
    ofstream init_file;
    init_file.open(root_directory_path + "/init_file");
    init_file << article_id << "\n";
    init_file << newsgroup_id << "\n";
    init_file.close();
}
            
int
DiskMemoryDataBase::get_id(string filename)
{
    regex rgx("^(\\d+)_.*$");
    smatch match;
    
    if (regex_search(filename, match, rgx)) {
        return stoi(match[1]);
    }
}
            
string
DiskMemoryDataBase::get_name(string filename)
{
    regex rgx("^\\d+_(.*)$");
    smatch match;
    
    if (regex_search(filename, match, rgx)) {
        return match[1];
    }
}

void
DiskMemoryDataBase::read_init()
{
    ifstream init_stream;
    init_stream.open(root_directory_path + "/init_file");
    string stringed_article_id;
    getline(init_stream, stringed_article_id);
    article_id = stoi(stringed_article_id); //First line = articles_id
                
    string stringed_newsgroup_id;
    getline(init_stream, stringed_newsgroup_id);
    newsgroup_id = stoi(stringed_newsgroup_id); //Second line = newsgroups_id
    init_stream.close();
}
            
            
