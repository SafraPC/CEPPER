const cepperListen = () => {
    const myInterval = setInterval(() => {
        let failOver = 0;
        const con = global.con;
        con.query("select query from tableQuery").then(response => {
            console.log("Response =>");
        }, (error) => {
            console.log("erru")
            if (failOver <= 7) {
                cepperListen();
            }
        })
        clearInterval(myInterval);
    }, 2000);
}
module.exports = { cepperListen }