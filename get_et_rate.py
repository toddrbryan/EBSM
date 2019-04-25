import os, sys, datetime, pytz, tzlocal, urllib.request, requests, csv, hashlib, json, boto3

uri = 'ftp://ftpcimis.water.ca.gov/pub2/daily/daily107.csv' #Station 107 is Santa Barbara
base_et = 0.15


def main():
  try:
    tempfile = tempfile_name()
    get_datafile(tempfile)
  except:
    print("Could not retrieve datafile " + tempfile)
    exit(-1)

  et = get_yesterdays_et(tempfile)
  if et == -1.0:
    print("No et found for " + datestr)
    exit(-1)

  new_water_level = int(et/base_et * 100)
  print("New Water Level will be %d" % new_water_level)
  status = set_os_et(new_water_level)
  notify(status)
  exit(0)

def yesterday():
  dt = datetime.datetime.now(datetime.timezone.utc)
  local_timezone = tzlocal.get_localzone()
  dt = dt.astimezone(local_timezone)
  delta = datetime.timedelta(1)
  dt = dt - delta
  return datetime.datetime.strftime(dt, "%-m/%-d/%Y")

def get_yesterdays_et(tempfile):
  datestr = yesterday()
  et = -1.0
  with open(tempfile, 'r') as tmp:
    rdr = csv.reader(tmp)
    for r in rdr:
      if r[1] == datestr:
        et = float(r[3])
        print("Found et for " + datestr + ": " + str(et))
  os.remove(tempfile)
  return et

def tempfile_name():
  return '/tmp/get_et_rate_' + str(datetime.datetime.now().timestamp()) + '.csv'

def get_datafile(tempfile):
  global uri
  urllib.request.urlretrieve(uri, tempfile)

def get_password():
  try:
    pw = os.environ['OPENSPRINKLER_PASSWORD']
  except:
    print("OpenSprinkler password not set in env variable OPENSPRINKLER_PASSWORD")
    exit(-1)
  pw = pw.encode('ascii')
  m = hashlib.md5()
  m.update(pw)
  return m.hexdigest()

def set_os_et(new_water_level):
  hash = get_password()
  status = ""
  r = requests.get('http://192.168.1.13/jo?pw=' + hash)
  res = json.loads(r.text)
  status = status + "Old water level: %s\n" % {res['wl']}
  r = requests.get('http://192.168.1.13/co?pw=%s&o23=%d' % (hash, new_water_level))
  r = requests.get('http://192.168.1.13/jo?pw=' + hash)
  res = json.loads(r.text)
  status = status + "Successfully set to new value %s\n" % {res['wl']}
  return status

def notify(status):
  session = boto3.Session(profile_name='trbryan')
  sns = session.client('sns')
  response = sns.publish(
    TopicArn='arn:aws:sns:us-west-2:509611857908:opensprinkler_et_update',
    Message=status,
    Subject='Daily OpenSprinkler ET Adjustment',
    MessageStructure='string',
  )


if __name__ == "__main__": main()