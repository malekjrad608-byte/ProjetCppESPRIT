from pathlib import Path
import re,sys
sys.path.insert(0,str(Path.cwd()/'tools'))
from check_model_sql_ownership import mask_cpp
def span(s,name):
 m=re.search(r"(?m)^.*\b"+re.escape(name)+r"\s*\(",s)
 if not m: raise ValueError(name)
 masked=mask_cpp(s);start=masked.index('{',m.start());depth=1;i=start+1
 while depth:
  depth+=(masked[i]=='{')-(masked[i]=='}');i+=1
 return m.start(),i
def extract(s,name):
 a,b=span(s,name);return s[a:b]
def replace(s,name,value):
 a,b=span(s,name);return s[:a]+value+s[b:]
