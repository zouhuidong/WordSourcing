///////////////////////////////////////////////////
//
//	WordSourcing
//	文本词源查找工具
// 
//	该程序可以将指定的单词表中的每个单词，
//	在指定的文章中进行溯源。
// 
//	该程序功能面向高中英语教材，项目目录下
//	的示例文件 (./res/*) 皆摘抄自
//	高中英语必修一教材 2019 统编版
// 
//	文件格式注意事项：
//	输入文件要求 ANSI 格式，否则中文会乱码；
//	从教材 pdf 文件中复制下来的内容默认为
//	Unicode 编码，需要手动转 ANSI，转换后
//	可能会出现很多问号，无需理会。
//	复制单词表时，注意不要把 "Unit 1" 一类的
//	文本复制上。复制时可能会复制到 pdf 文件中
//	右下角的水印，导致复制出一些多余且混乱的
//	文字，需要手动删除。请参考示例文件的格式。
//	复制文章内容时，直接复制下整个单元的文章
//	内容即可。
// 
//	该项目功能将合并到 Ted_Englishpia 中
// 
//	by huidong<huidong_mail@163.com>
//	2022.05.01
//


#include <stdio.h>
#include <conio.h>
#include <string>
#include <fstream>
#include <streambuf>
#include <vector>
#include <algorithm>
#include "EqualityChecker_DiffWordType.h"
using namespace std;

// 例句
struct ExampleSentence
{
	string en;	// 英文
	string zh;	// 中文
};

// 英语词典单词，包含一个单词应该具备的基本信息
typedef struct EnglishpiaWord
{
	string name;				// 单词
	string remark;              // 备注
	string pronunciation;		// 音标
	vector<string> meaning;     // 释义数组
	vector<ExampleSentence> sentences;   // 例句数组

}EWORD;

// 句子
struct Sentence
{
	string sentence;
	vector<string> words;
};

string strlwr(string s)
{
	transform(s.begin(), s.end(), s.begin(), ::tolower);
	return s;
}

string strupr(string s)
{
	transform(s.begin(), s.end(), s.begin(), ::toupper);
	return s;
}

// 获取文件内容
string GetFile(const char* filepath)
{
	ifstream t(filepath);
	string str((istreambuf_iterator<char>(t)), istreambuf_iterator<char>());
	t.close();

	if (str.size() <= 0)
	{
		printf("\n\nopen %s error.", filepath);
		_getch();
		exit(-1);
	}

	return str;
}

// 是否为英文字母（或空格）
bool isEnglishAlphabet(char c)
{
	return (c >= 65 && c <= 90) || (c >= 97 && c <= 122) || c == ' ';
}

// 换行符
bool isReturn(char c)
{
	return c == '\r' || c == '\n';
}

// 无用符
bool isUselessChar(char c)
{
	return isReturn(c) || c == '\t' || c == '?' /* 乱码 */;
}

// 中文
bool isChineseChar(char c)
{
	return c < 0;
}

// 判断接下来的内容是否为释义
bool isParaphrase(string str)
{
	for (int i = 0; i < (int)str.size(); i++)
	{
		if (!isEnglishAlphabet(str[i]))
		{
			if (str[i] == '.')
			{
				return true;
			}
			else if (isUselessChar(str[i]))
			{
			}
			else
			{
				return false;
			}
		}
	}
	return false;
}

// 读取单词备注
// 传入从 '(' 开始的字符串
string GetRemark(string res, int& index)
{
	string str;
	int i = 1;
	for (; i < (int)res.size() && res[i] != ')'; i++)
	{
		str += res[i];
	}
	index += i;
	return str;
}

// 跳过音标
// 传入从 '/' 开始的字符串
void SkipPhoneticSymbol(string res, int& index)
{
	int i = 1;
	for (; i < (int)res.size() && res[i] != '/'; i++);
	index += i;
}

// 读取释义
// 传入从释义开始的字符串
string GetParaphrase(string res, int& index)
{
	string str;
	int i = 0;
	for (; i < (int)res.size(); i++)
	{
		// 在读了一些释义的前提下，换行后直接出现英文
		// 判断该英文是否为释义的开头，如果不是则说明释义已经读完
		if (isUselessChar(res[i]))
		{
			// 空格乱码情况
			if (res[i] == '?')
			{
				str += ' ';
			}
			else if (str.size() > 2 && i + 1 < (int)res.size() &&
				isEnglishAlphabet(res[i + 1]) && res[i + 1] != ' ' && !isParaphrase(&res[i + 1]))
			{
				break;
			}
		}
		else if (res[i] == '/')
		{
			// 跳过音标
			SkipPhoneticSymbol(&res[i], i);
		}
		else
		{
			str += res[i];
		}
	}
	index += i;
	return str;
}

// 修剪字符串两侧的空白
string Trim(string str)
{
	bool flag = false;
	int size = (int)str.size(), end = size, k = 1;
	int i1 = 0, i2 = end;
	for (int i = 0;; i += k)
	{
		if ((k == 1 && i >= end) || (k == -1 && i < 0))
		{
			break;
		}
		if (str[i] == ' ')
		{
			if (k == 1)	i1 = i + 1;
			else		i2 = i;
			flag = true;
		}
		else if (flag || i == 0 || i == size - 1)
		{
			if (k == 1)
			{
				flag = false;
				i = end;
				end = 0;
				k = -1;
			}
			else
			{
				break;
			}
		}
	}
	return str.substr(i1, i2 - i1);
}

vector<EWORD> GetList(string strList)
{
	vector<EWORD> vecWords;
	EWORD word;
	bool end = false;
	int size = (int)strList.size();
	for (int i = 0; i < size; i++)
	{
		if (isEnglishAlphabet(strList[i]))
		{
			word.name += strList[i];
		}

		else if (isUselessChar(strList[i]))
		{

		}

		// 突然读到中文，说明直接进入释义
		else if (isChineseChar(strList[i]))
		{
			word.meaning.push_back(Trim(GetParaphrase(&strList[i], i)));
			end = true;
		}

		else
		{
			switch (strList[i])
			{
				// 单词内插入性备注
			case '(':
				word.remark = GetRemark(&strList[i], i);
				break;

			case '/':
				// 跳过音标
				SkipPhoneticSymbol(&strList[i], i);

				// 读完音标后，未换号，且出现英文，则可能是释义（英文词性），也可能还在单词中
				if (!(i + 1 < size && isUselessChar(strList[i + 1])))
				{
					if ((i += 2) < size && isEnglishAlphabet(strList[i]))
					{
						// 若接下来不是释义，就继续读单词
						if (!isParaphrase(&strList[i]))
						{
							break;
						}
					}
				}
				else
				{
					i += 2;
				}

				// 若接下来是释义，或直接出现中文，即可读取释义
				word.meaning.push_back(Trim(GetParaphrase(&strList[i], i)));
				end = true;
				break;
			}
		}

		if (end)
		{
			word.name = Trim(word.name);
			vecWords.push_back(word);
			word = {};
			end = false;
		}
	}
	return vecWords;
}

//////// 文章处理

// 符号匹配（可用于匹配 Unicode 字符）
bool isSymbol(string res, string sym)
{
	int s1 = (int)res.size(), s2 = (int)sym.size();
	if (s1 < s2)
	{
		return false;
	}
	for (int i = 0; i < s2; i++)
	{
		if (res[i] != sym[i])
		{
			return false;
		}
	}
	return true;
}

// 添加空格，并保证无重复空格
void AddWhitespace(string& str)
{
	int size = (int)str.size();
	if (size > 0 && str[size - 1] != ' ')
	{
		str += ' ';
	}
}

// 文章分词
vector<Sentence> AnalyzeArticle(string strArticle)
{
	vector<Sentence> vecSentences;
	Sentence sentence;
	string strWord;
	bool flagS = false, flagW = false;
	const int nSymbolsNum = 4;
	string strSymbols[nSymbolsNum] = { "‘","’","“","”" };

	for (int i = 0; i < (int)strArticle.size(); i++)
	{
		if (!isReturn(strArticle[i]))
		{
			if (strArticle[i] == ' ')
			{
				AddWhitespace(sentence.sentence);
			}
			else
			{
				sentence.sentence += strArticle[i];
			}
		}
		else
		{
			AddWhitespace(sentence.sentence);
		}

		// 歇止符判断
		for (int j = 0; j < nSymbolsNum; j++)
		{
			if (isSymbol(&strArticle[i], strSymbols[j]))
			{
				flagW = true;
				break;
			}
		}

		if (!flagW)
		{
			switch (strArticle[i])
			{
			case '.': case '?': case '!':
				flagS = true;
			case ' ': case ',': case ':': case';':
			case '(': case ')': case '/': case '\'': case '\"':
			case '\r': case '\n': case '\t':
				flagW = true;
				break;
			default:
				strWord += strArticle[i];
				break;
			}
		}

		if (flagW)
		{
			if (strWord.size() > 0)
			{
				sentence.words.push_back(strWord);
				strWord = "";
			}
			flagW = false;
		}
		if (flagS)
		{
			if (sentence.sentence.size() > 1)
			{
				vecSentences.push_back(sentence);
				sentence = {};
			}
			flagS = false;
		}
	}
	return vecSentences;
}

// 匹配句子
void MatchSentence(vector<EWORD>& vecList, vector<Sentence> vecSentences)
{
	for (int i = 0; i < (int)vecList.size(); i++)
	{
		if (vecList[i].name.find(" ") != vecList[i].name.npos)
		{
			continue;
		}
		for (int j = 0; j < (int)vecSentences.size(); j++)
		{
			for (int k = 0; k < (int)vecSentences[j].words.size(); k++)
			{
				if (strlwr(vecSentences[j].words[k]) == strlwr(vecList[i].name))
				{
					ExampleSentence es = { vecSentences[j].sentence };
					vecList[i].sentences.push_back(es);
					j = (int)1e9;
					break;
				}
			}
		}
	}
	for (int i = 0; i < (int)vecList.size(); i++)
	{
		if (vecList[i].name.find(" ") != vecList[i].name.npos)
		{
			continue;
		}
		if (vecList[i].sentences.size() == 0)
		{
			for (int j = 0; j < (int)vecSentences.size(); j++)
			{
				for (int k = 0; k < (int)vecSentences[j].words.size(); k++)
				{
					if (EChecker_DiffWT::CheckWordsEquality(strlwr(vecSentences[j].words[k]), strlwr(vecList[i].name)))
					{
						ExampleSentence es = { vecSentences[j].sentence };
						vecList[i].sentences.push_back(es);
						printf("%s -> %s\n", vecList[i].name.c_str(), vecSentences[j].words[k].c_str());
						j = (int)1e9;
						break;
					}
				}
			}
		}
	}
}


int main()
{
	EChecker_DiffWT::InitChecker(
		"./dic/irregular_nouns.txt",
		"./dic/irregular_verbs.txt",
		"./dic/prefix.txt",
		"./dic/suffix.txt"
	);

	string strList = GetFile("./res/U3/list.txt");
	string strArticle = GetFile("./res/U3/article.txt");

	vector<EWORD> vecWords = GetList(strList);
	vector<Sentence> vecSentences = AnalyzeArticle(strArticle);
	MatchSentence(vecWords, vecSentences);
	printf("\n");

	for (int i = 0; i < (int)vecWords.size(); i++)
	{
		printf("%d [%s (%s)]\n[%s]\n{%s}\n---\n\n",
			i, vecWords[i].name.c_str(), vecWords[i].remark.c_str(), vecWords[i].meaning[0].c_str(),
			vecWords[i].sentences.size() ? vecWords[i].sentences[0].en.c_str() : "");
	}

	_getch();
	return 0;
}
