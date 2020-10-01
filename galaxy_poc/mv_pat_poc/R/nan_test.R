
# check is R recognized NAN, NA

t1 <- 1.7976931348623158e+308
t2 <- Inf
t3 <- NA
t4 <- NaN
t5 <- "1.7976931348623158e+308"
t6 <- as.numeric(t5)
# t7 <- as.real(t5) could not find function "as.real"
t8 <- as.double(t5)

print("tlist")
tlist <- c(t1,t2,t3,t4,t6,t8)
print(tlist)
print("*** remove NaN ***")
tlistnona <- na.omit(tlist)
print(tlist)

save(tlist, tlistnona, file = "E:/galaxy_repositories/galaxy_dev_v72/galaxy_poc/mv_pat_poc/R/nan_test.RData")