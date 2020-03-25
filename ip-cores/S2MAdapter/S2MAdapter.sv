typedef logic [1:0] CsrAddr_t;
typedef logic [255:0] MasterData_t;
typedef logic [511:0] SinkData_t;
typedef logic [31:0] Word_t;

module S2MAdapter(
    input logic clock,
    input logic reset,
    
    // Avalon-MM slave interface for configuring the module
    input logic csr_write,
    input logic csr_read,
    input CsrAddr_t csr_address,
    input Word_t csr_writedata,
    output Word_t csr_readdata,
    
    // Avalon-MM master interface for outputting the data
    output logic m_write,
    output Word_t m_address,
    output MasterData_t m_writedata,
    output logic [1:0] m_burstcount,
    input logic m_waitrequest,
    
    // Avalon-ST sink interface for consuming the data
    input SinkData_t snk_data,
    input logic snk_valid,
    output logic snk_ready,
    
    // Interrupt request
    output logic irq
);
    // Control register adresses
    localparam CsrAddr_t LEN_ADDR = 2'h0;
    localparam CsrAddr_t ADDR_ADDR = 2'h1;
    localparam CsrAddr_t IRQ_ADDR = 2'h2;

    Word_t length;

    // Always use bursts of 2
    assign m_burstcount = 2'h2;
    
    // This needs to be turned into a reg
    // Probably skid buffer for data is needed
    assign snk_ready = length[0] == 1'b1 && (!m_waitrequest || !m_write);

    // Register read behavior
    always_ff @(posedge clock) begin        
        if (csr_read) begin
            case(csr_address)
                LEN_ADDR: csr_readdata <= length;
                ADDR_ADDR: csr_readdata <= m_address;
                IRQ_ADDR: csr_readdata <= irq;
            endcase
        end
    end
    
    always_ff @(posedge clock) begin
        // Reset logic
        if (reset) begin
            m_write <= 1'b0;
            length <= 1'b0;
        end
        
        // If registers are modified from csr interface
        else if (csr_write) begin
            if (csr_address == LEN_ADDR) begin
                length <= csr_writedata;
            end
        end
        
        // If we need to and can read more data from sink AND
        // If we can reed from sink AND
        // If the register where we place it is available
        if (length != 1'b0 && snk_valid && (!m_waitrequest || !m_write)) begin
            // One more 256-bit block will be processed
            length <= length - 1'b1;
            
            // Signalize that data is ready
            m_write <= 1'b1;
            
            // Choose which chunk to transfer
            if (length[0] == 1'b0) m_writedata <= snk_data[255:0];
            else m_writedata <= snk_data[511:256];
        end
        
        // If we cannot read data and
        // no write is pending, stop writing
        else if (!m_waitrequest) m_write <= 1'b0;
    end
    
    always_ff @(posedge clock) begin
        // Reset logic
        if (reset) begin
            irq <= 1'b0;
        end
        
        // If registers are modified from csr interface
        else if (csr_write) case(csr_address)
            ADDR_ADDR: m_address <= csr_writedata;
            IRQ_ADDR: irq <= 1'b0;
        endcase
        
        // If last transaction was successful
        if (!m_waitrequest && m_write) begin
            // 256 bits = 32 bytes
            m_address <= m_address + 32;

            // Send IRQ if this is the last transfer
            if (length == 1'b0) irq <= 1'b1;
        end
    end

endmodule