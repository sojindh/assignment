#댓글 크롤링
from time import sleep
from bs4 import BeautifulSoup
from selenium import webdriver
import os
import calendar
import requests
import csv
import re
import sys
import io
import pymysql
import pandas as pd
import operator
from konlpy.tag import Okt
import numpy as np

sys.stdout = io.TextIOWrapper(sys.stdout.detach(), encoding = 'utf-8')
sys.stderr = io.TextIOWrapper(sys.stderr.detach(), encoding = 'utf-8')

class SportCrawler:
    special_symbol = re.compile('[\{\}\[\]\/?,;:|\)*~`!^\-_+<>@\#$&▲▶◆◀■【】\\\=\(\'\"]')
    content_pattern = re.compile('본문 내용|TV플레이어| 동영상 뉴스|flash 오류를 우회하기 위한 함수 추가function  flash removeCallback|tt|앵커 멘트|xa0')

    def __init__(self):
        self.date={'startyear':0, 'endyear':0, 'endmonth':0}

    def totalpage(self, url):
        totalpage_url=url
        driver = webdriver.Chrome("C:\\Users\\dhdqu\\Desktop\\chromedriver_win32\\chromedriver")
        #페이지가 동적할당인 것 같아서 셀레니움을 활용하였다.
        driver.get(url)
        html=driver.page_source
        soup=BeautifulSoup(html)
        prodList= soup.select('#_pageList > a')
        sleep(2)
        driver.close()
        return (len(prodList)+1)

    def clear_content(self, text):
        # 기사 본문에서 필요없는 특수문자 및 본문 양식 등을 다 지움
        newline_symbol_removed_text = text.replace('\\n', '').replace('\\t', '').replace('\\r', '')
        special_symbol_removed_content = re.sub(self.special_symbol, ' ', newline_symbol_removed_text)
        end_phrase_removed_content = re.sub(self.content_pattern, '', special_symbol_removed_content)
        blank_removed_content = re.sub(' +', ' ', end_phrase_removed_content).lstrip()  # 공백 에러 삭제
        reversed_content = ''.join(reversed(blank_removed_content))  # 기사 내용을 reverse 한다.
        content = ''
        for i in range(0, len(blank_removed_content)):
            # reverse 된 기사 내용중, ".다"로 끝나는 경우 기사 내용이 끝난 것이기 때문에 기사 내용이 끝난 후의 광고, 기자 등의 정보는 다 지움
            if reversed_content[i:i + 2] == '.다':
                content = ''.join(reversed(reversed_content[i:]))
                break
        return content

    def Make_url(self, URL, startyear, startmonth, startday):
        Maked_url = []
        endDay=[0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31]
        month=startmonth
        day=startday
        for i in range(0,7):
            url=URL
            date=str(startyear)
            if len(str(month))==1:
                str_month="0"+str(month)
            elif len(str(month))==2:
                str_month=str(month)
            if len(str(day))==1:
                str_day="0"+str(day)
            elif len(str(day))==2:
                str_day=str(day)
            date=date+str_month+str_day
            if((day+1)>endDay[month]):
                if(month==12):
                    month=1
                    startyear+=1
                else :
                    month+=1
                    day=1
            else:
                day+=1
            url=url+date
            totalpage = self.totalpage(url)  # TotalPage 확인
            for page in range(1, totalpage + 1):
                url=url+"&page="+str(page)
                Maked_url.append(url)
        return Maked_url

    def parse(self):
        driver = webdriver.Chrome("C:\\Users\\dhdqu\\Desktop\\chromedriver_win32\\chromedriver")
        driver.implicitly_wait(3)
        conn=pymysql.connect(host='localhost', user='root', password='1234', db='maru')
        curs=conn.cursor()
        url="https://sports.news.naver.com/wfootball/news/index.nhn?isphoto=N&type=comment&date="
        final_urlday=[]
        while(len(final_urlday)==0):
            final_urlday=self.Make_url(url,2019, 11, 6)#각 페이지까지 접근
        print("succed making url")

        for URL in final_urlday:#날짜까지만 접근
            driver.get(URL)
            html=driver.page_source
            soup=BeautifulSoup(html)
            tag_document=soup.select('#_newsList > ul > li > div > a')#페이지의 각 리스트까지 접근
            reple_post=[]
            head_post=[]
            for tag in tag_document:#뉴스페이지 접근
                driver.get('https://sports.news.naver.com'+tag.get('href'))
                reple_post.append('https://sports.news.naver.com'+tag.get('href')+'&m_view=1&sort=LIKE')#댓글창까지 접근
                head_post.append('https://sports.news.naver.com'+tag.get('href'))
            for reple_url, head_url in zip(reple_post, head_post):
                CONTENTS=pd.read_sql_query("SELECT * FROM blog_name", conn)
                dic=dict(CONTENTS)
                sent=[]
                main_word={}

                driver.get(head_url)
                html=driver.page_source
                document_content=BeautifulSoup(html, 'html.parser')
                try:
                    tag_headline=document_content.find_all('h4', {'class':'title'})
                    text_headline=''
                    text_headline=tag_headline[0].string
                    special_symbol_removed_headline = re.sub('[\{\}\[\]\/?,;:|\)*~`!^\-_+<>@\#$%&n▲▶◆◀■\\\=\(\'\"]', '', text_headline)
                    text_headline=re.sub("'", '', text_headline);
                    if not special_symbol_removed_headline:
                        continue
                    tag_content=document_content.find_all('div', {'id':'newsEndContents'})
                    text_content=''
                    text_content = text_content + self.clear_content(str(tag_content[0].find_all(text=True)))

                    sent.append(text_headline)
                    sent.append(text_content)
                    okt=Okt()

                    for sentence in sent:
                        data=okt.pos(sentence)
                        for j in data:
                            if(j[1]=='Noun'):
                                if(j[0]=='일본'):
                                    raise Exception
                                for k in range(0, 1795):
                                    if(dic['name'][k]==j[0]):
                                        if (dic['dictionary_id'][k] in main_word):
                                            main_word[dic['dictionary_id'][k]]+=1
                                        else:
                                            main_word[dic['dictionary_id'][k]]=1
                    sorted_freqlist=sorted(main_word.items(), key=operator.itemgetter(1), reverse=True)
                    date=URL[83:87]+'-'+URL[87:89]+'-'+URL[89:91]
                    sql="insert into blog_news values ( null, '"+text_headline+"', '"+date+"', "+str(sorted_freqlist[0][0])+");"
                    curs.execute(sql)
                    conn.commit()
                    driver.get(reple_url)
                    total_page=3
                    while total_page:
                        try:
                            click_button=driver.find_element_by_xpath('''//*[@id="cbox_module"]/div/div[8]/a''')
                            click_button.click()
                            total_page-=1
                        except Exception as e:
                            break
                    sleep(2)
                    html=driver.page_source
                    document_content=BeautifulSoup(html, 'html.parser')
                    try:
                        reple=document_content.find_all('span',{'class':'u_cbox_contents'})
                        for i in range(0, len(reple)):
                            flag1=0
                            flag2=0
                            if (len(reple[i].string)>95):
                                continue
                            for k in range(0, 1795):
                                if flag2==1:
                                    break
                                if((dic['dictionary_id'][k]==sorted_freqlist[0][0]) and (dic['name'][k] in reple[i].string)): # 그 단어가 문장에서 들어갈때
                                    flag1=1
                                if((dic['dictionary_id'][k]!=sorted_freqlist[0][0]) and (dic['name'][k] in reple[i].string)): # 그 단어가 문장에 들어가는데, 중심단어는 아님
                                    flag2=1
                            if(("쪽바리" in reple[i].string) or ("기자" in reple[i].string) or ("일본" in reple[i].string) or ("기레기" in reple[i].string)):
                                continue
                            elif((flag1==1) and (flag2==0)):
                                sql="insert into blog_comm values ( null, '"+reple[i].string+"', 0, '"+date+"', "+str(sorted_freqlist[0][0])+");"
                                curs.execute(sql)
                                conn.commit()
                            elif((flag1==0) and(flag2==0)):
                                sql="insert into blog_comm values ( null, '"+reple[i].string+"', 0, '"+date+"', "+str(sorted_freqlist[0][0])+");"
                                curs.execute(sql)
                                conn.commit()
                    except Exception as ex:
                        pass
                except Exception as e:
                    pass
        driver.close()


if __name__=="__main__":
    Cralwer=SportCrawler()
    Cralwer.parse()
