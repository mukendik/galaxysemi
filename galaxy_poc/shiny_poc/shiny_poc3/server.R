
# This is the server logic for a Shiny web application.
# You can find out more about building applications with Shiny here:
# 
# http://www.rstudio.com/shiny/
#

library(shiny)
library(DBI)
library(RMySQL)
require(rCharts)
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

  output$chart <- renderChart({
    p1 <- rPlot(input$x, input$y, data = iris, color = "Species", facet = "Species", type = 'point')
    p1$addParams(dom = 'myChart')
    return(p1)
  })
  
})

#dbDisconnect(con)
#print("Server: disconnected.")
