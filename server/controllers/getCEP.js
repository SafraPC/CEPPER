const axios = require("axios");
const getCep = async (cep) => {
	try {
		console.log("achei o cep!! ", cep);
		const { data } = await axios.get(`https://viacep.com.br/ws/${cep}/json/`);
		const con = global.con;
		if (data) {
			console.log(data);
			if (!data.erro) {
				con.query(
					`insert into ceps(logradouro,bairro,localidade,uf,ibge,gia,ddd,siafi) values('${data.logradouro}', '${data.bairro}', '${data.localidade}', '${data.uf}', '${data.ibge}', '${data.gia}', '${data.ddd}', '${data.siafi}');`
				);
			}
		}
	} catch (error) {
		console.log("Catch getCEP error =>", error);
	}
};
module.exports = { getCep };
