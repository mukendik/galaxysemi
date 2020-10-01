
# This is the server logic for a Shiny web application.
# You can find out more about building applications with Shiny here:
# 
# http://www.rstudio.com/shiny/
#

library(shiny)
library(DBI)
library(RMySQL)
# library(knitr)

#con <- dbConnect(MySQL(), user="root", password="root", dbname="gexdb_3390")

cat("Server: current DB exception: ", length( dbGetException(con) ), "\n")

#res <- dbSendQuery(con, "select * from wt_ptest_results where splitlot_id=1011000012 and ptest_info_id=1")

# 

bins <- 40

shinyServer(function(input, output) 
{
  
  # output
  #output$uppercase <- renderText({"qsdfqs"})
  output$distPlot <- renderPlot({
    cat("renderPlot for test input$dataset", input$dataset, "\n")
  
    #
    index_of_testname<-match(input$dataset, test_infos[,2])
    cat("Index of ", input$dataset, "=",index_of_testname, "\n" )
    query<-paste("select value from wt_ptest_results where splitlot_id=1011000012 and ptest_info_id=", index_of_testname)
    res <- dbSendQuery(con, query)
    x<-NULL
    x <- fetch(res, n=-1) # in datasamples: 800 test results
    cat ("Fetched: ", dbGetRowCount(res), "\n")
    cat("DB exception: ", length( dbGetException(con) ), "\n")
    # dbClearResult(res) # ?
    cat("Retrieved: ", length(x[,1]), " testresults\n")
    m<-mean(x[,1])
    cat("mean", m, "\n")
    #cat("dim x", dim(x), "\n")
    #cat("head x", head(x[,1]), "\n")
    
    #output$dist<-x
    
    # generate bins based on input$bins from ui.R
    #x    <- faithful[, 2] 
    
    #bins <- seq(min(x), max(x), length.out = input$bins + 1)
    
    
    if (input$charttype=="histogram")
    {
      # draw the histogram with the specified number of bins
      hist(x[,1], breaks=bins, col='green', border = 'white', freq=FALSE, 
           main = input$dataset,
           xlab="Test results", ylab="Distribution (%)")
      abline(NULL, 0, v=m, col="red")
    }
    else
    {
      #plot( seq(length=dbGetRowCount(res), from=0, by=1), x[,1])
      plot(seq(length=length(x[,1]), from=0, by=1), x[,1], 
           main=paste("Test ", input$dataset),
           type="l", col="green",  ylab="Test results", 
           xlab=paste("Data count (", length(x[,1]), " samples found)" ) )
      abline(m, 0, col="red")
    }
    
  })
})

#dbDisconnect(con)
#print("Server: disconnected.")
