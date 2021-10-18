const comumQuery = (req, res) => {
  try {
    const { query } = req.body;
    const con = global.con;
    let resposta;
    let requestError;
    con.query(query).then(
      (response) => {
        resposta = response[0];
      },
      (error) => {
        requestError = true;
        resposta = error;
      }
    );
    setTimeout(() => {
      if (requestError) {
        return res.send({ success: false, ...resposta });
      }
      return res.send(resposta);
    }, 1500);
  } catch (error) {
    console.log(error);
    return res
      .status(400)
      .send({ message: "Não foi possível enviar resposta!" });
  }
};

module.exports = { comumQuery };
