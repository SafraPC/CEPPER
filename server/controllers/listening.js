const { getCep } = require("./getCEP");

const cepperListen = () => {
	setInterval(() => {
		const con = global.con;
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
