#include "seqPatch2_headers.h"


/*-------------------internal functions---------------------*/
bool loadContextHyperPara(SEXP R_context_hyperPara, SEXP R_context, map<string, map<string, double> > &context_hyperPara)
{
	if (Rf_length(R_context_hyperPara)!=Rf_length(R_context))
                printf("loadContextHyperPara : length of R_context_hyperPara and R_context should be the same.\n");
	
	for (int i=0;i<Rf_length(R_context_hyperPara);i++){
                        string cur_context = CHAR(STRING_ELT(R_context,i));
                        map<string, double> cur_data;
			SEXP R_cur_data = VECTOR_ELT(R_context_hyperPara,i);
                        
			// get data
                        cur_data["theta"] = REAL(VECTOR_ELT(R_cur_data,getIdxByName(R_cur_data, "theta")))[0];
               		cur_data["kappa"] = REAL(VECTOR_ELT(R_cur_data,getIdxByName(R_cur_data, "kappa")))[0];
			cur_data["upsilon"] = REAL(VECTOR_ELT(R_cur_data,getIdxByName(R_cur_data, "upsilon")))[0];
			cur_data["tau2"] = REAL(VECTOR_ELT(R_cur_data,getIdxByName(R_cur_data, "tau2")))[0];
			 
			// load to context_hyperPara 
			context_hyperPara[cur_context] = cur_data;
                }
	
	return true;
}

/*-----------------------R API-----------------------*/
RcppExport SEXP R_API_BayesianMixtureModel (SEXP R_IPD, SEXP R_idx, SEXP R_theta_0, SEXP R_kappa_0, SEXP R_upsilon_0, SEXP R_tau2_0, 
				SEXP R_theta_1, SEXP R_kappa_1, SEXP R_upsilon_1, SEXP R_tau2_1, SEXP R_max_iter)
{
	BayesianMixtureModel_NC BayesianMixtureModelObj;
	BayesianMixtureModelObj.setHyperParametersNull(REAL(R_theta_0)[0], REAL(R_kappa_0)[0], REAL(R_upsilon_0)[0], REAL(R_tau2_0)[0]);
	BayesianMixtureModelObj.getMoleculeMeanIPD(REAL(R_IPD), REAL(R_idx), Rf_length(R_IPD), Rf_length(R_idx));
	BayesianMixtureModelObj.run(INTEGER(R_max_iter)[0]);		
	

	return Rcpp::List::create(Rcpp::Named("theta_0_t")=Rcpp::wrap(BayesianMixtureModelObj.get_theta_0_t_track()),
				Rcpp::Named("kappa_0_t")=Rcpp::wrap(BayesianMixtureModelObj.get_kappa_0_t_track()),
	 			Rcpp::Named("upsilon_0_t")=Rcpp::wrap(BayesianMixtureModelObj.get_upsilon_0_t_track()),
				Rcpp::Named("tau2_0_t")=Rcpp::wrap(BayesianMixtureModelObj.get_tau2_0_t_track()),
				Rcpp::Named("N_0_t")=Rcpp::wrap(BayesianMixtureModelObj.get_N_0_t_track()),
				Rcpp::Named("N_gamma_0_t")=Rcpp::wrap(BayesianMixtureModelObj.get_N_gamma_0_t_track()),

				Rcpp::Named("theta_1_t")=Rcpp::wrap(BayesianMixtureModelObj.get_theta_1_t_track()),
                                Rcpp::Named("kappa_1_t")=Rcpp::wrap(BayesianMixtureModelObj.get_kappa_1_t_track()),
                                Rcpp::Named("upsilon_1_t")=Rcpp::wrap(BayesianMixtureModelObj.get_upsilon_1_t_track()),
                                Rcpp::Named("tau2_1_t")=Rcpp::wrap(BayesianMixtureModelObj.get_tau2_1_t_track()),
                                Rcpp::Named("N_1_t")=Rcpp::wrap(BayesianMixtureModelObj.get_N_1_t_track()),
                                Rcpp::Named("N_gamma_1_t")=Rcpp::wrap(BayesianMixtureModelObj.get_N_gamma_1_t_track()),
		
				Rcpp::Named("gamma_0")=Rcpp::wrap(BayesianMixtureModelObj.get_gamma_0()),
				Rcpp::Named("gamma_1")=Rcpp::wrap(BayesianMixtureModelObj.get_gamma_1()));		

      	return R_NilValue;
}

RcppExport SEXP R_API_DetectModProp_NC(SEXP R_IPD, SEXP R_idx, SEXP R_genome_start, SEXP R_genomeSeq, SEXP R_context_hyperPara,
				 SEXP R_context, SEXP R_strand, SEXP R_left_len, SEXP R_right_len, SEXP R_max_iter)
{
	/*---------------------check validation of inputs---------------------*/
	if (Rf_length(R_IPD)!=Rf_length(R_idx)){
		Rprintf("inconsistent IPD and moleculeID.\n");
		return R_NilValue;
	}	


	/*---------------- load data and parameters -----------------------*/
	
	// load context_hyperPara
	Rprintf("load context effect hyperparameters.\n");
	map<string, map<string, double> > context_hyperPara;
	loadContextHyperPara(R_context_hyperPara, R_context, context_hyperPara);
	
	// load parameters
	int genome_start = INTEGER(R_genome_start)[0];
	string genomeSeq = CHAR(STRING_ELT(R_genomeSeq,0));
	int strand = INTEGER(R_strand)[0];
	int left_len = 	INTEGER(R_left_len)[0];
	int right_len = INTEGER(R_right_len)[0];
	int max_iter = INTEGER(R_max_iter)[0];

	/*------------ initialized results that need to be recorded ----------*/
	
	vector<double> is_findContext(Rf_length(R_IPD), sqrt(-1));	
	
	// prior distribution parameters ( " *_0 " for null distribution and " *_1 " for alternative(modified) distribution )		
	vector<double> theta_0(Rf_length(R_IPD), sqrt(-1)); 
	vector<double> kappa_0(Rf_length(R_IPD), sqrt(-1)); 
	vector<double> upsilon_0(Rf_length(R_IPD), sqrt(-1)); 
	vector<double> tau2_0(Rf_length(R_IPD), sqrt(-1));

	// posterior distribution parameters 
	vector<double> theta_0_t(Rf_length(R_IPD), sqrt(-1)); vector<double> theta_1_t(Rf_length(R_IPD), sqrt(-1));
        vector<double> kappa_0_t(Rf_length(R_IPD), sqrt(-1)); vector<double> kappa_1_t(Rf_length(R_IPD), sqrt(-1));
        vector<double> upsilon_0_t(Rf_length(R_IPD), sqrt(-1)); vector<double> upsilon_1_t(Rf_length(R_IPD), sqrt(-1));
        vector<double> tau2_0_t(Rf_length(R_IPD), sqrt(-1)); vector<double> tau2_1_t(Rf_length(R_IPD), sqrt(-1));

	// number of iteration, coverge 
	vector<double> n_steps(Rf_length(R_IPD), sqrt(-1));
	vector<double> cvg(Rf_length(R_IPD), sqrt(-1));	

	/*----------------- run ------------------*/
	Rprintf("run.\n");
	BayesianMixtureModel_NC BayesianMixtureModelObj;
	int genome_size = Rf_length(R_IPD);
	for (int i=0;i<genome_size;i++){
		if ((i+1)%10000==0) Rprintf("detected %d positions\r", i+1);

		// check data and load IPD and moleculeID
		if (Rf_length(VECTOR_ELT(R_IPD,i)) != Rf_length(VECTOR_ELT(R_idx,i))){
			Rprintf("in the %dth position: inconsistent IPD and moleculeID length.\n",i+1);
			return R_NilValue;
		}
		vector<double> cur_IPD(REAL(VECTOR_ELT(R_IPD,i)), REAL(VECTOR_ELT(R_IPD,i)) + Rf_length(VECTOR_ELT(R_IPD,i)));
		vector<double> cur_idx(REAL(VECTOR_ELT(R_idx,i)), REAL(VECTOR_ELT(R_idx,i)) + Rf_length(VECTOR_ELT(R_idx,i)));
		
		// find context of current position
		int cur_pos = i + genome_start - 1;
		string cur_context;
		if (strand == 0){
			if (cur_pos - left_len<0 || cur_pos + right_len>Rf_length(R_IPD)-1 || cur_pos + right_len > (int) genomeSeq.size()-1)
				continue;
			cur_context = genomeSeq.substr(cur_pos - left_len , right_len + left_len + 1);		
		}else{
			if (cur_pos - right_len<0 || cur_pos + left_len>Rf_length(R_IPD)-1 || cur_pos + left_len > (int) genomeSeq.size()-1)
                                continue;
			cur_context = SP_reverseSeq (genomeSeq.substr(cur_pos - right_len, right_len + left_len + 1) ) ;
		}
		
		if (cur_IPD.size()<3){
                	cvg[i] = cur_IPD.size();
                	continue;
        	}
		
		// find context hyperparameters of current position
		map<string, map<string, double> >::iterator it = context_hyperPara.find(cur_context);
               	if (it != context_hyperPara.end()){
			is_findContext[i] = 1;
		}else{
			is_findContext[i] = 0;
			continue;	
		}

		// load hyperparameters of current position
		theta_0[i] = it->second["theta"];
		kappa_0[i] = it->second["kappa"];
		upsilon_0[i] = it->second["upsilon"];
		tau2_0[i] = it->second["tau2"];
		
		// fit Bayesian Mixture Model	
		BayesianMixtureModelObj.setHyperParametersNull(theta_0[i], kappa_0[i], upsilon_0[i], tau2_0[i]);
		BayesianMixtureModelObj.getMoleculeMeanIPD(&cur_IPD[0], &cur_idx[0], cur_IPD.size(), cur_idx.size());
		BayesianMixtureModelObj.run(max_iter);
		
		// get results
							
				
	}
	Rprintf("detected %d positions\n", genome_size);

	return Rcpp::wrap(context_hyperPara);	
        return R_NilValue;
}


RcppExport SEXP R_API_hieModel_core(SEXP R_IPD_mean, SEXP R_IPD_var, SEXP R_IPD_n, SEXP R_max_iter)
{
	vector<double> IPD_mean(REAL(R_IPD_mean), REAL(R_IPD_mean) + Rf_length(R_IPD_mean) );
        vector<double> IPD_var(REAL(R_IPD_var), REAL(R_IPD_var) + Rf_length(R_IPD_var) );
        vector<double> IPD_n(REAL(R_IPD_n), REAL(R_IPD_n) + Rf_length(R_IPD_n) );
	int max_iter = INTEGER(R_max_iter)[0];
	
	return Rcpp::wrap(hieModelEB(IPD_mean, IPD_var, IPD_n, max_iter) );	
}

 


