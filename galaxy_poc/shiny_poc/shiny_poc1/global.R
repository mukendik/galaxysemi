print("Global")
print("Global: Connecting...")
con <- dbConnect(MySQL(), user="root", password="root", dbname="gexdb_3390")
for(e in dbGetException(con))
{
  print(e)
}

test_infos_res<-dbSendQuery(con, 
            "select ptest_info_id, concat(tnum, ' ', tname) from wt_ptest_info where splitlot_id=1011000012")
test_infos<-fetch(test_infos_res, n=-1)
cat("global: ", length(test_infos[,1]), " tests info found\n")
