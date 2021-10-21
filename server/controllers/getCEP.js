const axios = require("axios");
const getCep = async (cep) => {
	const con = global.con;
	try {
		console.log("achei o cep!! ", cep);
		const { data } = await axios.get(`https://viacep.com.br/ws/${cep}/json/`);
		if (data) {
			if (!data.erro) {
				con.query(
					`insert into ceps(cep,logradouro,bairro,localidade,uf,ibge,gia,ddd,siafi) values('${cep}','${data.logradouro}', '${data.bairro}', '${data.localidade}', '${data.uf}', '${data.ibge}', '${data.gia}', '${data.ddd}', '${data.siafi}');`
				);
			}
		}
	} catch (error) {
		con.query(`truncate table tberror`);
		con.query(`insert into tberror(error) values('erro')`);
	}
};
module.exports = { getCep };
