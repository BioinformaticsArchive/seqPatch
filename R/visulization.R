plotAvgIPD.density <- function(IPD, mol.id, IPD.ctrl=NULL, mol.id.ctrl=NULL, loci,
			 ref.genome, strand, context.hyperPara, out.file, tl='density', left.len =6, right.len = 1)
{
	if (length(IPD) != length(mol.id)) 
		stop('inconsistant IPD and mol.id.')
	IPD.list <- split(IPD, mol.id)
	IPD.avg <- sapply(IPD.list, mean)
	if (!is.null(IPD.ctrl) & !is.null(mol.id.ctrl)){
		if (length(IPD.ctrl) != length(mol.id.ctrl))
	                stop('inconsistant IPD.ctrl and mol.id.ctrl.')
		IPD.list.ctrl <- split(IPD.ctrl, mol.id.ctrl)
        	IPD.avg.ctrl <- sapply(IPD.list.ctrl, mean)
	
		
	}
	
	if (strand  == 0){
		cur.context <- substr(ref.genome, loci - left.len, loci + right.len)
	}else{
		cur.context <- reverseSeq(substr(ref.genome, loci - right.len, loci + left.len))
	}
	cur.hyperPara <- context.hyperPara[[cur.context]]
	dens <- density(IPD.avg)
	x <- seq(cur.hyperPara$theta - 5*sqrt(cur.hyperPara$tau2), 
		cur.hyperPara$theta + 5*sqrt(cur.hyperPara$tau2), length.out=500)
        y <- dnorm(x, cur.hyperPara$theta, sqrt(cur.hyperPara$tau2) )
	
	if (!is.null(IPD.ctrl) & !is.null(mol.id.ctrl))	{
		dens.ctrl <- density(IPD.avg.ctrl)
		png(file=out.file, type='cairo')
                plot(dens, col='red', xlab='average IPD', xlim=range(x,dens$x, dens.ctrl$x),
			ylim = range(y, dens$y, dens.ctrl$y), main=tl)
                lines(dens.ctrl, col='green')
		lines(x,y, col = 'blue', lty=2)
		dev.off()	

	}else{
		png(file=out.file, type='cairo')
		plot(dens, col='red', xlab='average IPD', xlim=range(x,dens$x),ylim = range(y, dens$y),
			main=tl)
		lines(x,y, col = 'blue', lty=2)	
		dev.off()
	}	
	
}




