const { getCep } = require("./getCEP");

const cepperListen = () => {
	setInterval(() => {
		let failOver = 0;
		const con = global.con;
		//insert into tbquery(query) values('13057050');
		//select * from ceps
		//truncate table ceps
		con.query("select * from tbquery").then(
			(response) => {
				if (response[0][0]) {
					getCep(response[0][0].query + "");
					con.query("truncate table tbquery");
					return;
				}
			},
			(error) => {
				console.log("error");
			}
		);
	}, 2000);
};
module.exports = { cepperListen };
