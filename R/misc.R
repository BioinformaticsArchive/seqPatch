getR2OfContextEffect <- function(context.effect.mean)
{
	mean.all <- sum(sapply(context.effect.mean, sum)) / sum(sapply(context.effect.mean, length))
	var.all <- sum(sapply(context.effect.mean, function(x,m) sum((x-m)^2), m=mean.all))	
	var.err <- sum(sapply(context.effect.mean, function(x) sum((x-mean(x))^2)))	
	1 - var.err/var.all
}

combine.genomeSeq <- function(genomeSeq.1, genomeSeq.2)
{
	genomeSeq <- genomeSeq.1
	genomeSeq$pos <- c(genomeSeq.1$pos, genomeSeq.2$pos)	
        genomeSeq$neg <- c(genomeSeq.1$neg, genomeSeq.2$neg)
	genomeSeq
}

combine.genomeF <- function(genomeF.1, genomeF.2)
{
	genomeF <- genomeF.1
	genomeF$features$ipd_pos <- c(genomeF.1$features$ipd_pos, genomeF.2$features$ipd_pos)	
        genomeF$features$ipd_neg <- c(genomeF.1$features$ipd_neg, genomeF.2$features$ipd_neg)
	genomeF$genome.start.pos <- c(genomeF.1$genome.start.pos, genomeF.2$genome.start.pos)
        genomeF$genome.start.neg <- c(genomeF.1$genome.start.neg, genomeF.2$genome.start.neg)
	genomeF
}


outlier.rm <- function(x,k=2.25)
{
	tmp <- quantile(x,prob=c(0.25,0.75))
	inter.len <- tmp[2] - tmp[1]
	upper.bd <- tmp[2] + k*inter.len
	lower.bd <- tmp[1] - k*inter.len
	x[x <=upper.bd]
}
bc.transform <- function(x,lambda)
{
	(x^lambda-1)/lambda
}

refresh <- function()
{
	dyn.unload(paste(system.file(package = "seqPatch"), "/libs/seqPatch.so",sep = ""))
	dyn.load(paste(system.file(package = "seqPatch"), "/libs/seqPatch.so",sep = ""))
}
reverseSeq <- function(seq)
{
        .Call('reverseSeq',seq)
}


trim.context.effect <- function(context.effect.bypos, cvg.cutoff = 20, context.ex=NULL)
{
        ce.len <- sapply(context.effect.bypos, function(x) sapply(x,length))
	if (is.null(context.ex))
        	return (mapply( function(x,len,cutoff){ x[len>=cutoff]  } ,context.effect.bypos, ce.len, cutoff = cvg.cutoff ))
	else{
		if (any(names(context.effect.bypos)!=names(context.ex)))
			stop('imcompatible context.effect.bypos and context.ex\n')
		for (i in 1:length(context.effect.bypos)){
			if (i==1000*floor(i/1000))cat('filtered ',i,' contexts\r')
			context.effect.bypos[[i]] <- context.effect.bypos[[i]][ce.len[[i]]>=cvg.cutoff]
			context.ex[[i]] <- context.ex[[i]][ce.len[[i]]>=cvg.cutoff]
		}
		cat('filtered ', length(context.effect.bypos), ' contexts\n')
		return (list(context.effect=context.effect.bypos,context.ex=context.ex))
	}
	
}


filter.outlier <- function(x)
{
	.Call('filter_outlier_wrap',as.numeric(x))
}

filter.outlier.byGenomeF <- function(ipd)
{
	.Call('filter_outlier_by_genomeF',ipd)
}

logTransGenomeF <- function(genomeF)
{
	for (i in 1:length(genomeF$features$ipd_pos))
                genomeF$features$ipd_pos[[i]] <- sapply(genomeF$features$ipd_pos[[i]], function(x) log(x+0.01) )
        for (i in 1:length(genomeF.native$features$ipd_neg))
                genomeF$features$ipd_neg[[i]] <- sapply(genomeF$features$ipd_neg[[i]], function(x) log(x+0.01) )
	genomeF
}


CollapseContextEffectByPos <- function(context.effect.bypos, cvg.cutoff = 20, is.log=TRUE)
{
	rl <- .Call('collapseContextEffectByPos', context.effect.bypos, as.integer(cvg.cutoff))	
	rl <- as.data.frame(rl)
	names(rl)[1] <- 'ipd.log'
	names(rl)[2:ncol(rl)] <- paste('c',1:(ncol(rl)-1),sep='')
	for (i in 2:ncol(rl)) rl[,i] <- as.factor(rl[,i])
	rl
}

get.subread.R2 <- function(data, is.log = TRUE)
{
	if (is.log==TRUE) for (i in 1:length(data)) data[[i]]$IPD <- log(data[[i]]$IPD+0.01)
	
	### get total variance 
	S <- sapply(data, function(x) sum(x$IPD,na.rm=TRUE))
	len <- sapply(data,nrow)
	S.mean <- sum(S)/sum(len)
	
	T <- sum(sapply(data, function(x,S.mean) sum((x$IPD-S.mean)^2,na.rm=TRUE), S.mean=S.mean))

	### get variance within group
	W <- sum(sapply(data, function(x) sum((x$IPD - mean(x$IPD,na.rm=TRUE))^2 ,na.rm=TRUE)))	
	(T - W)/T	
}


plot.cmp.context.effect <- function(cmp.context.effect, outfile='~/cmp_contexteffect.png')
{
        data <- as.data.frame(cbind(cmp.context.effect$x, cmp.context.effect$y))
        names(data) <- c('x','y')
        model <- lm(y~x,data=data)

        png(file=outfile,type='cairo')
        min.sig<- min(cmp.context.effect$x, cmp.context.effect$y)
        max.sig<- max(cmp.context.effect$x, cmp.context.effect$y)
        plot(cmp.context.effect$x, cmp.context.effect$y, pch=19,cex=0.2, col='blue', xlim=c(min.sig, max.sig), ylim=c(min.sig, max.sig), xlab='context effect E. Coli WGA', ylab='context effect MP WGA ', main=paste('PCC=',cor(cmp.context.effect$x,cmp.context.effect$y),sep=''))
        abline(model$coef[1], model$coef[2],col='red')
        abline(0,1,col='green')
        dev.off()


}




getOverDisp <- function(x)
{
        bd <- quantile(x,prob=c(0.01,0.99) )
        y <- x[x>bd[1] & x<bd[2]]
        return ( sd(y)/mean(abs(y)))

}

getSd.filter <- function(x, is.log=FALSE)
{
	y <- filter.outlier(x, method='high')
	if (is.log==TRUE){
		return(sd(log(y+0.01)))	
	}else{
		return(sd(y))
	}
}

sub.sample.reads <- function(alnsF, alnsIdx , ratio)
{
        idx.sel <- sample(1:length(alnsF), size=floor(length(alnsF)*ratio+0.5))
        alnsF.sel <- alnsF[idx.sel]
        alnsIdx.sel <- alnsIdx[idx.sel, ]
        result <- list()
        result$alnsF <- alnsF.sel
        result$alnsIdx <- alnsIdx.sel
        result
}

