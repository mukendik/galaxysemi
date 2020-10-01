
# This is the user-interface definition of a Shiny web application.
# You can find out more about building applications with Shiny here:
# 
# http://www.rstudio.com/shiny/
#

library(shiny)
library(DBI)
library(RMySQL)

#print("UI: Connecting to DB...")

#res <- dbSendQuery(con, "select ss.hbin_no, hb.hbin_name, ss.bin_count from wt_hbin_stats_summary ss join wt_hbin hb on ss.hbin_no=hb.hbin_no where ss.bin_count>0 group by ss.hbin_no")
#res <- dbSendQuery(con, "select * from wt_ptest_results where splitlot_id=1011000012 and ptest_info_id=1")

distinct_test_info<-dbSendQuery(con, "select distinct ptest_info_id from wt_ptest_results where splitlot_id=1011000012")
out<-NULL
while(!dbHasCompleted( distinct_test_info ))
{
  #f<- fetch(res, n=1)
  #chunk <- fetch(res, n = 10000)
  #out <- c(out, doit(chunk))
  #out<-c(out, chunk)
  out<- fetch(distinct_test_info, n=10000)
}


shinyUI(  
  fluidPage(
  )
    
#   pageWithSidebar(
#     # Application title
#     headerPanel("Interactive mode"),
#     #headerPanel(con),
#     # Sidebar with a slider input for number of bins
#     #sidebarPanel(
#     #  sliderInput("bins","Number of bins:", min = 1, max = 50, value = 30)
#     #),
#     sidebarPanel( 
#       #selectInput("dataset", "Test info ID:", choices = c( out  ))
#       selectInput("dataset", "Test:", choices = c( test_infos[,2] )),
#       selectInput("charttype", "Chart:", choices = c( "histogram", "trend" ))
#     ),
#     # Show a plot of the generated distribution
#     mainPanel(
#       #textOutput("dist")
#       plotOutput("distPlot")
#     )
#   ) # pageWithSidebar
  
) # shinyUI

#dbDisconnect(con)
#print("UI: disconnected.")